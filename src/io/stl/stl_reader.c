#include "stl_reader.h"
#include "../core/geometry/vec3.h"
#include "../core/geometry/brep.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

TopoShape* stl_import_binary(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }
    STLHeader header;
    size_t read = fread(&header, sizeof(STLHeader), 1, file);
    if (read != 1) {
        fclose(file);
        fprintf(stderr, "Erreur: Impossible de lire l'en-tete STL\n");
        return NULL;
    }
    if (header.num_triangles == 0) {
        fclose(file);
        fprintf(stderr, "Erreur: Aucun triangle dans le fichier STL\n");
        return NULL;
    }
    STLTriangle* triangles = (STLTriangle*)malloc(header.num_triangles * sizeof(STLTriangle));
    if (!triangles) {
        fclose(file);
        fprintf(stderr, "Erreur: Impossible d'allouer la memoire pour les triangles\n");
        return NULL;
    }
    read = fread(triangles, sizeof(STLTriangle), header.num_triangles, file);
    if (read != header.num_triangles) {
        free(triangles);
        fclose(file);
        fprintf(stderr, "Erreur: Impossible de lire tous les triangles\n");
        return NULL;
    }
    fclose(file);
    TopoShape* shape = NULL;
    AABB aabb = AABB_EMPTY;
    for (uint32_t i = 0; i < header.num_triangles; i++) {
        Vec3 v1 = {{triangles[i].vertex1[0], triangles[i].vertex1[1], triangles[i].vertex1[2]}};
        Vec3 v2 = {{triangles[i].vertex2[0], triangles[i].vertex2[1], triangles[i].vertex2[2]}};
        Vec3 v3 = {{triangles[i].vertex3[0], triangles[i].vertex3[1], triangles[i].vertex3[2]}};
        aabb = aabb_merge(aabb, aabb_from_points((const Vec3[]){v1, v2, v3}, 3));
    }
    shape = topo_solid_create();
    TopoShape* shell = topo_shell_create();
    topo_shape_add_child(shape, shell);
    for (uint32_t i = 0; i < header.num_triangles; i++) {
        Vec3 v1 = {{triangles[i].vertex1[0], triangles[i].vertex1[1], triangles[i].vertex1[2]}};
        Vec3 v2 = {{triangles[i].vertex2[0], triangles[i].vertex2[1], triangles[i].vertex2[2]}};
        Vec3 v3 = {{triangles[i].vertex3[0], triangles[i].vertex3[1], triangles[i].vertex3[2]}};
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
    free(triangles);
    topo_shape_update_aabb(shape);
    return shape;
}

TopoShape* stl_import_ascii(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }
    char line[256];
    uint32_t num_triangles = 0;
    STLTriangle* triangles = NULL;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "facet normal") || strstr(line, "FACET NORMAL")) {
            num_triangles++;
        }
    }
    if (num_triangles == 0) {
        fclose(file);
        fprintf(stderr, "Erreur: Aucun triangle dans le fichier STL ASCII\n");
        return NULL;
    }
    triangles = (STLTriangle*)malloc(num_triangles * sizeof(STLTriangle));
    if (!triangles) {
        fclose(file);
        fprintf(stderr, "Erreur: Impossible d'allouer la memoire pour les triangles\n");
        return NULL;
    }
    rewind(file);
    uint32_t index = 0;
    while (fgets(line, sizeof(line), file) && index < num_triangles) {
        if (strstr(line, "facet normal") || strstr(line, "FACET NORMAL")) {
            if (sscanf(line, "facet normal %f %f %f", &triangles[index].normal[0], &triangles[index].normal[1], &triangles[index].normal[2]) != 3) {
                if (sscanf(line, "FACET NORMAL %f %f %f", &triangles[index].normal[0], &triangles[index].normal[1], &triangles[index].normal[2]) != 3) {
                    free(triangles);
                    fclose(file);
                    fprintf(stderr, "Erreur: Impossible de lire la normale du triangle %u\n", index);
                    return NULL;
                }
            }
            if (!fgets(line, sizeof(line), file) || !(strstr(line, "outer loop") || strstr(line, "OUTER LOOP"))) {
                free(triangles);
                fclose(file);
                fprintf(stderr, "Erreur: 'outer loop' attendu apres la normale\n");
                return NULL;
            }
            for (int i = 0; i < 3; i++) {
                if (!fgets(line, sizeof(line), file) || !(strstr(line, "vertex") || strstr(line, "VERTEX"))) {
                    free(triangles);
                    fclose(file);
                    fprintf(stderr, "Erreur: 'vertex' attendu pour le sommet %d\n", i);
                    return NULL;
                }
                float* vertex = NULL;
                switch (i) {
                    case 0: vertex = triangles[index].vertex1; break;
                    case 1: vertex = triangles[index].vertex2; break;
                    case 2: vertex = triangles[index].vertex3; break;
                }
                if (sscanf(line, "vertex %f %f %f", &vertex[0], &vertex[1], &vertex[2]) != 3) {
                    if (sscanf(line, "VERTEX %f %f %f", &vertex[0], &vertex[1], &vertex[2]) != 3) {
                        free(triangles);
                        fclose(file);
                        fprintf(stderr, "Erreur: Impossible de lire le sommet %d\n", i);
                        return NULL;
                    }
                }
            }
            if (!fgets(line, sizeof(line), file) || !(strstr(line, "endloop") || strstr(line, "ENDLOOP"))) {
                free(triangles);
                fclose(file);
                fprintf(stderr, "Erreur: 'endloop' attendu\n");
                return NULL;
            }
            if (!fgets(line, sizeof(line), file) || !(strstr(line, "endfacet") || strstr(line, "ENDFACET"))) {
                free(triangles);
                fclose(file);
                fprintf(stderr, "Erreur: 'endfacet' attendu\n");
                return NULL;
            }
            index++;
        }
    }
    fclose(file);
    TopoShape* shape = NULL;
    AABB aabb = AABB_EMPTY;
    for (uint32_t i = 0; i < num_triangles; i++) {
        Vec3 v1 = {{triangles[i].vertex1[0], triangles[i].vertex1[1], triangles[i].vertex1[2]}};
        Vec3 v2 = {{triangles[i].vertex2[0], triangles[i].vertex2[1], triangles[i].vertex2[2]}};
        Vec3 v3 = {{triangles[i].vertex3[0], triangles[i].vertex3[1], triangles[i].vertex3[2]}};
        aabb = aabb_merge(aabb, aabb_from_points((const Vec3[]){v1, v2, v3}, 3));
    }
    shape = topo_solid_create();
    TopoShape* shell = topo_shell_create();
    topo_shape_add_child(shape, shell);
    for (uint32_t i = 0; i < num_triangles; i++) {
        Vec3 v1 = {{triangles[i].vertex1[0], triangles[i].vertex1[1], triangles[i].vertex1[2]}};
        Vec3 v2 = {{triangles[i].vertex2[0], triangles[i].vertex2[1], triangles[i].vertex2[2]}};
        Vec3 v3 = {{triangles[i].vertex3[0], triangles[i].vertex3[1], triangles[i].vertex3[2]}};
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
    free(triangles);
    topo_shape_update_aabb(shape);
    return shape;
}

bool stl_is_binary(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }
    uint8_t header[80];
    size_t read = fread(header, 1, 80, file);
    bool is_ascii = true;
    for (size_t i = 0; i < read; i++) {
        if (header[i] == 0 || (header[i] < 32 && header[i] != '\n' && header[i] != '\r' && header[i] != '\t')) {
            is_ascii = false;
            break;
        }
    }
    fclose(file);
    return !is_ascii;
}

bool stl_is_ascii(const char* filename) {
    return !stl_is_binary(filename);
}

TopoShape* stl_import(const char* filename) {
    if (stl_is_binary(filename)) {
        return stl_import_binary(filename);
    } else {
        return stl_import_ascii(filename);
    }
}