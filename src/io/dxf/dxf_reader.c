#include "dxf_reader.h"
#include "../modeling/sketch/Sketch.hpp"
#include "../modeling/sketch/SketchEntity.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    DXF_LINE = 1,
    DXF_CIRCLE = 2,
    DXF_ARC = 3,
    DXF_TEXT = 4,
    DXF_POINT = 5,
    DXF_LWPOLYLINE = 100,
    DXF_3DFACE = 105
} DXFEntityType;

typedef struct {
    DXFEntityType type;
    Vec3 start;
    Vec3 end;
    Vec3 center;
    float radius;
    float start_angle;
    float end_angle;
    int num_vertices;
    Vec3* vertices;
} DXFEntity;

Sketch* dxf_import(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }
    Sketch* sketch = new Sketch("DXF_Import", SketchPlane::XY);
    char line[256];
    DXFEntity entity = {0};
    bool in_entity = false;
    while (fgets(line, sizeof(line), file)) {
        int code;
        if (sscanf(line, "%d", &code) != 1) continue;
        switch (code) {
            case 0:
                if (strstr(line, "LINE")) { entity.type = DXF_LINE; in_entity = true; }
                else if (strstr(line, "CIRCLE")) { entity.type = DXF_CIRCLE; in_entity = true; }
                else if (strstr(line, "ARC")) { entity.type = DXF_ARC; in_entity = true; }
                else if (strstr(line, "POINT")) { entity.type = DXF_POINT; in_entity = true; }
                else if (strstr(line, "LWPOLYLINE")) { entity.type = DXF_LWPOLYLINE; in_entity = true; }
                else if (strstr(line, "ENDSEC") || strstr(line, "EOF")) { in_entity = false; }
                break;
            case 10: if (in_entity) { entity.start.x = atof(line + 2); if (entity.type == DXF_LWPOLYLINE && entity.num_vertices > 0) entity.vertices[entity.num_vertices - 1].x = atof(line + 2); } break;
            case 20: if (in_entity) { entity.start.y = atof(line + 2); if (entity.type == DXF_LWPOLYLINE && entity.num_vertices > 0) entity.vertices[entity.num_vertices - 1].y = atof(line + 2); } break;
            case 30: if (in_entity) { entity.start.z = atof(line + 2); if (entity.type == DXF_LWPOLYLINE && entity.num_vertices > 0) entity.vertices[entity.num_vertices - 1].z = atof(line + 2); } break;
            case 11: if (in_entity && entity.type == DXF_LINE) entity.end.x = atof(line + 2); break;
            case 21: if (in_entity && entity.type == DXF_LINE) entity.end.y = atof(line + 2); break;
            case 31: if (in_entity && entity.type == DXF_LINE) entity.end.z = atof(line + 2); break;
            case 40: if (in_entity && (entity.type == DXF_CIRCLE || entity.type == DXF_ARC)) entity.radius = atof(line + 2); break;
            case 50: if (in_entity && entity.type == DXF_ARC) entity.start_angle = atof(line + 2) * M_PI / 180.0f; break;
            case 51: if (in_entity && entity.type == DXF_ARC) entity.end_angle = atof(line + 2) * M_PI / 180.0f; break;
            case 90: if (in_entity && entity.type == DXF_LWPOLYLINE) { entity.num_vertices = atoi(line + 2); entity.vertices = (Vec3*)malloc(entity.num_vertices * sizeof(Vec3)); } break;
            case 70: break;
        }
        if (in_entity && code == 0 && strstr(line, "LINE") == NULL && strstr(line, "CIRCLE") == NULL && strstr(line, "ARC") == NULL && strstr(line, "POINT") == NULL && strstr(line, "LWPOLYLINE") == NULL) {
            switch (entity.type) {
                case DXF_LINE: sketch->addLine(entity.start, entity.end); break;
                case DXF_CIRCLE: sketch->addCircle(entity.start, entity.radius); break;
                case DXF_ARC: sketch->addArc(entity.start, entity.radius, entity.start_angle, entity.end_angle); break;
                case DXF_POINT: sketch->addPoint(entity.start); break;
                case DXF_LWPOLYLINE:
                    for (int i = 0; i < entity.num_vertices - 1; i++) sketch->addLine(entity.vertices[i], entity.vertices[i + 1]);
                    if (entity.num_vertices > 0) { free(entity.vertices); entity.vertices = NULL; entity.num_vertices = 0; }
                    break;
            }
            in_entity = false;
        }
    }
    fclose(file);
    if (entity.vertices) free(entity.vertices);
    return sketch;
}

bool dxf_import_to_sketch(const char* filename, Sketch* sketch) {
    if (!sketch) return false;
    Sketch* imported_sketch = dxf_import(filename);
    if (!imported_sketch) return false;
    for (auto& entity : imported_sketch->getEntities()) sketch->getEntities().push_back(std::move(entity));
    delete imported_sketch;
    return true;
}