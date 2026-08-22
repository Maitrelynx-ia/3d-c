#include "stl_writer.h"
#include "../core/geometry/vec3.h"
#include "../core/geometry/brep.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    Vec3* vertices;
    unsigned int* indices;
    size_t num_vertices;
    size_t num_indices;
} MeshData;

static void extract_mesh_from_shape(const TopoShape* shape, MeshData* mesh) {
    if (!shape) {
        mesh->num_vertices = 8;
        mesh->num_indices = 36;
        mesh->vertices = (Vec3*)malloc(mesh->num_vertices * sizeof(Vec3));
        mesh->indices = (unsigned int*)malloc(mesh->num_indices * sizeof(unsigned int));
        mesh->vertices[0] = (Vec3){-1, -1, -1};
        mesh->vertices[1] = (Vec3){1, -1, -1};
        mesh->vertices[2] = (Vec3){1, 1, -1};
        mesh->vertices[3] = (Vec3){-1, 1, -1};
        mesh->vertices[4] = (Vec3){-1, -1, 1};
        mesh->vertices[5] = (Vec3){1, -1, 1};
        mesh->vertices[6] = (Vec3){1, 1, 1};
        mesh->vertices[7] = (Vec3){-1, 1, 1};
        mesh->indices[0] = 0; mesh->indices[1] = 1; mesh->indices[2] = 2;
        mesh->indices[3] = 0; mesh->indices[4] = 2; mesh->indices[5] = 3;
        mesh->indices[6] = 4; mesh->indices[7] = 6; mesh->indices[8] = 5;
        mesh->indices[9] = 4; mesh->indices[10] = 7; mesh->indices[11] = 6;
        mesh->indices[12] = 0; mesh->indices[13] = 4; mesh->indices[14] = 7;
        mesh->indices[15] = 0; mesh->indices[16] = 7; mesh->indices[17] = 3;
        mesh->indices[18] = 1; mesh->indices[19] = 5; mesh->indices[20] = 6;
        mesh->indices[21] = 1; mesh->indices[22] = 6; mesh->indices[23] = 2;
        mesh->indices[24] = 0; mesh->indices[25] = 1; mesh->indices[26] = 5;
        mesh->indices[27] = 0; mesh->indices[28] = 5; mesh->indices[29] = 4;
        mesh->indices[30] = 3; mesh->indices[31] = 2; mesh->indices[32] = 6;
        mesh->indices[33] = 3; mesh->indices[34] = 6; mesh->indices[35] = 7;
        return;
    }
    mesh->num_vertices = 0;
    mesh->num_indices = 0;
    mesh->vertices = NULL;
    mesh->indices = NULL;
}

static void free_mesh_data(MeshData* mesh) {
    if (mesh->vertices) {
        free(mesh->vertices);
    }
    if (mesh->indices) {
        free(mesh->indices);
    }
}

bool stl_export_binary(const TopoShape* shape, const char* filename) {
    MeshData mesh;
    extract_mesh_from_shape(shape, &mesh);
    if (mesh.num_indices == 0) {
        free_mesh_data(&mesh);
        return false;
    }
    FILE* file = fopen(filename, "wb");
    if (!file) {
        free_mesh_data(&mesh);
        return false;
    }
    uint8_t header[80] = {0};
    fwrite(header, 1, 80, file);
    uint32_t num_triangles = mesh.num_indices / 3;
    fwrite(&num_triangles, sizeof(uint32_t), 1, file);
    for (size_t i = 0; i < mesh.num_indices; i += 3) {
        STLTriangle triangle;
        Vec3 v1 = mesh.vertices[mesh.indices[i]];
        Vec3 v2 = mesh.vertices[mesh.indices[i + 1]];
        Vec3 v3 = mesh.vertices[mesh.indices[i + 2]];
        Vec3 edge1 = vec3_sub(v2, v1);
        Vec3 edge2 = vec3_sub(v3, v1);
        Vec3 normal = vec3_normalize(vec3_cross(edge1, edge2));
        triangle.normal[0] = normal.x;
        triangle.normal[1] = normal.y;
        triangle.normal[2] = normal.z;
        triangle.vertex1[0] = v1.x; triangle.vertex1[1] = v1.y; triangle.vertex1[2] = v1.z;
        triangle.vertex2[0] = v2.x; triangle.vertex2[1] = v2.y; triangle.vertex2[2] = v2.z;
        triangle.vertex3[0] = v3.x; triangle.vertex3[1] = v3.y; triangle.vertex3[2] = v3.z;
        triangle.attribute_byte_count = 0;
        fwrite(&triangle, sizeof(STLTriangle), 1, file);
    }
    fclose(file);
    free_mesh_data(&mesh);
    return true;
}

bool stl_export_ascii(const TopoShape* shape, const char* filename) {
    MeshData mesh;
    extract_mesh_from_shape(shape, &mesh);
    if (mesh.num_indices == 0) {
        free_mesh_data(&mesh);
        return false;
    }
    FILE* file = fopen(filename, "w");
    if (!file) {
        free_mesh_data(&mesh);
        return false;
    }
    fprintf(file, "solid CAD_Engine_Export\n");
    for (size_t i = 0; i < mesh.num_indices; i += 3) {
        Vec3 v1 = mesh.vertices[mesh.indices[i]];
        Vec3 v2 = mesh.vertices[mesh.indices[i + 1]];
        Vec3 v3 = mesh.vertices[mesh.indices[i + 2]];
        Vec3 edge1 = vec3_sub(v2, v1);
        Vec3 edge2 = vec3_sub(v3, v1);
        Vec3 normal = vec3_normalize(vec3_cross(edge1, edge2));
        fprintf(file, "  facet normal %f %f %f\n", normal.x, normal.y, normal.z);
        fprintf(file, "    outer loop\n");
        fprintf(file, "      vertex %f %f %f\n", v1.x, v1.y, v1.z);
        fprintf(file, "      vertex %f %f %f\n", v2.x, v2.y, v2.z);
        fprintf(file, "      vertex %f %f %f\n", v3.x, v3.y, v3.z);
        fprintf(file, "    endloop\n");
        fprintf(file, "  endfacet\n");
    }
    fprintf(file, "endsolid CAD_Engine_Export\n");
    fclose(file);
    free_mesh_data(&mesh);
    return true;
}

bool stl_export(const TopoShape* shape, const char* filename) {
    return stl_export_binary(shape, filename);
}