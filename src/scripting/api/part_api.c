#include "part_api.h"
#include "../modeling/part/Part.hpp"
#include "../modeling/sketch/Sketch.hpp"
#include "../modeling/assembly/Assembly.hpp"
#include "../io/serialization/cad_json.h"
#include "../io/serialization/cad_binary.h"
#include "../io/stl/stl_writer.h"
#include <string.h>

Part* part_new(const char* name) { return new Part(name ? name : "Unnamed"); }
void part_delete(Part* part) { delete part; }
void part_set_name(Part* part, const char* name) { if (part && name) part->setName(name); }
const char* part_get_name(const Part* part) { if (!part) return ""; return part->getName().c_str(); }

Sketch* part_create_sketch(Part* part, const char* plane_type) {
    if (!part || !plane_type) return NULL;
    SketchPlane plane;
    if (strcmp(plane_type, "XY") == 0) plane = SketchPlane::XY;
    else if (strcmp(plane_type, "XZ") == 0) plane = SketchPlane::XZ;
    else if (strcmp(plane_type, "YZ") == 0) plane = SketchPlane::YZ;
    else plane = SketchPlane::XY;
    return part->createSketch(plane);
}

Sketch* part_create_sketch_on_plane(Part* part, float ox, float oy, float oz, float nx, float ny, float nz) {
    if (!part) return NULL; Plane plane; plane.origin = (Vec3){ox, oy, oz}; plane.normal = (Vec3){nx, ny, nz}; return part->createSketch(plane);
}

void sketch_add_line(Sketch* sketch, float x1, float y1, float z1, float x2, float y2, float z2) {
    if (!sketch) return; Vec3 start = (Vec3){x1, y1, z1}; Vec3 end = (Vec3){x2, y2, z2}; sketch->addLine(start, end);
}

void sketch_add_circle(Sketch* sketch, float x, float y, float z, float radius) {
    if (!sketch) return; Vec3 center = (Vec3){x, y, z}; sketch->addCircle(center, radius);
}

void sketch_add_arc(Sketch* sketch, float x, float y, float z, float radius, float start_angle, float end_angle) {
    if (!sketch) return; Vec3 center = (Vec3){x, y, z}; float start_rad = start_angle * M_PI / 180.0f; float end_rad = end_angle * M_PI / 180.0f; sketch->addArc(center, radius, start_rad, end_rad);
}

void sketch_add_point(Sketch* sketch, float x, float y, float z) {
    if (!sketch) return; Vec3 position = (Vec3){x, y, z}; sketch->addPoint(position);
}

void part_extrude(Part* part, Sketch* sketch, float depth, bool is_additive) { if (!part || !sketch) return; part->addExtrude(sketch, depth, is_additive); }
void part_revolve(Part* part, Sketch* sketch, float angle, float ax, float ay, float az) { if (!part || !sketch) return; Vec3 axis = (Vec3){ax, ay, az}; float angle_rad = angle * M_PI / 180.0f; part->addRevolve(sketch, angle_rad, axis); }
void part_add_fillet(Part* part, float radius) { if (!part) return; }
void part_add_chamfer(Part* part, float distance) { if (!part) return; }
void part_add_hole(Part* part, Sketch* sketch, float depth, bool is_through) { if (!part || !sketch) return; part->addHole(sketch, depth, is_through); }
void part_boolean_union(Part* part1, Part* part2) {}
void part_boolean_difference(Part* part1, Part* part2) {}
void part_boolean_intersection(Part* part1, Part* part2) {}

Assembly* assembly_new(const char* name) { return new Assembly(name ? name : "Unnamed"); }
void assembly_delete(Assembly* assembly) { delete assembly; }
void assembly_add_part(Assembly* assembly, Part* part) { if (!assembly || !part) return; assembly->addPart(part); }
void assembly_remove_part(Assembly* assembly, Part* part) { if (!assembly || !part) return; assembly->removePart(part); }
void assembly_add_coincident_constraint(Assembly* assembly, Part* part1, Part* part2) {}
void assembly_add_distance_constraint(Assembly* assembly, Part* part1, Part* part2, float distance) {}
void assembly_add_parallel_constraint(Assembly* assembly, Part* part1, Part* part2) {}
void assembly_add_perpendicular_constraint(Assembly* assembly, Part* part1, Part* part2) {}

bool assembly_solve_constraints(Assembly* assembly) { if (!assembly) return false; return assembly->solveConstraints(); }
bool assembly_detect_collisions(Assembly* assembly) { if (!assembly) return false; return assembly->hasCollisions(); }
bool assembly_save(Assembly* assembly, const char* filename) { if (!assembly || !filename) return false; return cad_save_json(assembly, filename); }
Assembly* assembly_load(const char* filename) { if (!filename) return NULL; return cad_load_json(filename); }

bool assembly_export_stl(Assembly* assembly, const char* filename) {
    if (!assembly || !filename) return false;
    for (Part* part : assembly->getParts()) {
        TopoShape* body = part->getBody();
        if (body) { std::string part_filename = std::string(filename) + "_" + part->getName() + ".stl"; stl_export(body, part_filename.c_str()); }
    }
    return true;
}

bool assembly_export_step(Assembly* assembly, const char* filename) { if (!assembly || !filename) return false; return false; }
