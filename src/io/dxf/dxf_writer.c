#include "dxf_writer.h"
#include "../modeling/sketch/Sketch.hpp"
#include "../modeling/sketch/SketchEntity.hpp"
#include <stdio.h>

bool dxf_export(const Sketch* sketch, const char* filename) {
    if (!sketch) return false;
    FILE* file = fopen(filename, "w");
    if (!file) return false;
    fprintf(file, "  999\n");
    fprintf(file, "DXF created by CAD Engine\n");
    fprintf(file, "\n");
    fprintf(file, "  2\n");
    fprintf(file, "ENTITIES\n");
    for (const auto& entity : sketch->getEntities()) {
        switch (entity->type) {
            case SketchEntityType::LINE: {
                SketchLine* line = static_cast<SketchLine*>(entity.get());
                fprintf(file, "  0\nLINE\n  8\n0\n  10\n%f\n  20\n%f\n  30\n%f\n  11\n%f\n  21\n%f\n  31\n%f\n",
                    line->start.x, line->start.y, line->start.z, line->end.x, line->end.y, line->end.z);
                break;
            }
            case SketchEntityType::CIRCLE: {
                SketchCircle* circle = static_cast<SketchCircle*>(entity.get());
                fprintf(file, "  0\nCIRCLE\n  8\n0\n  10\n%f\n  20\n%f\n  30\n%f\n  40\n%f\n",
                    circle->center.x, circle->center.y, circle->center.z, circle->radius);
                break;
            }
            case SketchEntityType::ARC: {
                SketchArc* arc = static_cast<SketchArc*>(entity.get());
                fprintf(file, "  0\nARC\n  8\n0\n  10\n%f\n  20\n%f\n  30\n%f\n  40\n%f\n  50\n%f\n  51\n%f\n",
                    arc->center.x, arc->center.y, arc->center.z, arc->radius,
                    arc->start_angle * 180.0f / M_PI, arc->end_angle * 180.0f / M_PI);
                break;
            }
            case SketchEntityType::POINT: {
                SketchPoint* point = static_cast<SketchPoint*>(entity.get());
                fprintf(file, "  0\nPOINT\n  8\n0\n  10\n%f\n  20\n%f\n  30\n%f\n",
                    point->position.x, point->position.y, point->position.z);
                break;
            }
            default: break;
        }
    }
    fprintf(file, "  0\nENDSEC\n  0\nEOF\n");
    fclose(file);
    return true;
}