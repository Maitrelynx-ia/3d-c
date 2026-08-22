#include "obj_reader.h"
#include "../core/geometry/vec3.h"
#include "../core/geometry/brep.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    Vec3* vertices;
    Vec3* normals;
    Vec3* texcoords;
    unsigned int* indices;
    size_t num_vertices;
    size_t num_normals;
    size_t num_texcoords;
    size_t num_indices;
    size_t num_faces;
} OBJData;

static void free_obj_data(OBJData* obj) {
    if (obj->vertices) free(obj->vertices);
    if (obj->normals) free(obj->normals);
    if (obj->texcoords) free(obj->texcoords);
    if (obj->indices) free(obj->indices);
}

TopoShape* obj_import(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }
    OBJData obj = {0};
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') obj.num_vertices++;
            else if (line[1] == 'n') obj.num_normals++;
            else if (line[1] == 't') obj.num_texcoords++;
        } else if (line[0] == 'f') {
            obj.num_faces++;
            obj.num_indices += 3;
        }
    }
    obj.vertices = (Vec3*)calloc(obj.num_vertices, sizeof(Vec3));
    obj.normals = (Vec3*)calloc(obj.num_normals, sizeof(Vec3));
    obj.texcoords = (Vec3*)calloc(obj.num_texcoords, sizeof(Vec3));
    obj.indices = (unsigned int*)malloc(obj.num_indices * sizeof(unsigned int));
    if (!obj.vertices || !obj.normals || !obj.texcoords || !obj.indices) {
        free_obj_data(&obj);
        fclose(file);
        fprintf(stderr, "Erreur: Impossible d'allouer la memoire\n");
        return NULL;
    }
    rewind(file);
    size_t v_index = 0;
    size_t vn_index = 0;
    size_t vt_index = 0;
    size_t f_index = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') {
                if (sscanf(line, "v %f %f %f", &obj.vertices[v_index].x, &obj.vertices[v_index].y, &obj.vertices[v_index].z) == 3) {
                    v_index++;
                }
            } else if (line[1] == 'n') {
                if (sscanf(line, "vn %f %f %f", &obj.normals[vn_index].x, &obj.normals[vn_index].y, &obj.normals[vn_index].z) == 3) {
                    vn_index++;
                }
            } else if (line[1] == 't') {
                float w = 0.0f;
                if (sscanf(line, "vt %f %f %f", &obj.texcoords[vt_index].x, &obj.texcoords[vt_index].y, &w) >= 2) {
                    obj.texcoords[vt_index].z = w;
                    vt_index++;
                }
            }
        } else if (line[0] == 'f') {
            char* token = line + 2;
            char* endptr;
            unsigned int indices[3][3] = {0};
            int count = 0;
            while (*token && count < 3) {
                indices[count][0] = strtoul(token, &endptr, 10);
                if (endptr == token) break;
                token = endptr;
                if (*token == '/') {
                    token++;
                    if (*token != '/') {
                        indices[count][1] = strtoul(token, &endptr, 10);
                        token = endptr;
                    }
                }
                if (*token == '/') {
                    token++;
                    indices[count][2] = strtoul(token, &endptr, 10);
                    token = endptr;
                }
                count++;
            }
            if (count == 3) {
                obj.indices[f_index++] = indices[0][0] - 1;
                obj.indices[f_index++] = indices[1][0] - 1;
                obj.indices[f_index++] = indices[2][0] - 1;
            }
        }
    }
    fclose(file);
    TopoShape* shape = topo_solid_create();
    TopoShape* shell = topo_shell_create();
    topo_shape_add_child(shape, shell);
    for (size_t i = 0; i < obj.num_indices; i += 3) {
        Vec3 v1 = obj.vertices[obj.indices[i]];
        Vec3 v2 = obj.vertices[obj.indices[i + 1]];
        Vec3 v3 = obj.vertices[obj.indices[i + 2]];
        TopoShape* vertex1 = topo_vertex_create(v1);
        TopoShape* vertex2 = topo_vertex_create(v2);
        TopoShape* vertex3 = topo_vertex_create(v3);
        TopoShape* edge1 = topo_edge_create(vertex1, vertex2, EDGE_LINE, NULL);
        TopoShape* edge2 = topo_edge_create(vertex2, vertex3, EDGE_LINE, NULL);
        TopoShape* edge3 = topo_edge_create(vertex3, vertex1, EDGE_LINE, NULL);
        TopoShape* wire = topo_wire_create();
        topo_shape_add_child(wire, edge1);
        topo_shape_add_child(wire, edge2);
        topo_shape_add_child(wire, edge3);
        Plane* plane = (Plane*)malloc(sizeof(Plane));
        plane->origin = v1;
        plane->normal = vec3_normalize(vec3_cross(vec3_sub(v2, v1), vec3_sub(v3, v1)));
        TopoShape* face = topo_face_create(FACE_PLANE, plane);
        topo_face_data(face)->outer_wire = wire;
        topo_shape_add_child(shell, face);
    }
    free_obj_data(&obj);
    topo_shape_update_aabb(shape);
    return shape;
}