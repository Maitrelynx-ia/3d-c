#ifndef CAD_PART_API_H
#define CAD_PART_API_H

#include "../modeling/part/Part.hpp"
#include "../modeling/sketch/Sketch.hpp"
#include "../modeling/assembly/Assembly.hpp"

#ifdef __cplusplus
extern "C" {
#endif

Part* part_new(const char* name);
void part_delete(Part* part);
void part_set_name(Part* part, const char* name);
const char* part_get_name(const Part* part);
Sketch* part_create_sketch(Part* part, const char* plane_type);
Sketch* part_create_sketch_on_plane(Part* part, float ox, float oy, float oz, float nx, float ny, float nz);
void sketch_add_line(Sketch* sketch, float x1, float y1, float z1, float x2, float y2, float z2);
void sketch_add_circle(Sketch* sketch, float x, float y, float z, float radius);
void sketch_add_arc(Sketch* sketch, float x, float y, float z, float radius, float start_angle, float end_angle);
void sketch_add_point(Sketch* sketch, float x, float y, float z);
void part_extrude(Part* part, Sketch* sketch, float depth, bool is_additive);
void part_revolve(Part* part, Sketch* sketch, float angle, float ax, float ay, float az);
void part_add_fillet(Part* part, float radius);
void part_add_chamfer(Part* part, float distance);
void part_add_hole(Part* part, Sketch* sketch, float depth, bool is_through);
void part_boolean_union(Part* part1, Part* part2);
void part_boolean_difference(Part* part1, Part* part2);
void part_boolean_intersection(Part* part1, Part* part2);
Assembly* assembly_new(const char* name);
void assembly_delete(Assembly* assembly);
void assembly_add_part(Assembly* assembly, Part* part);
void assembly_remove_part(Assembly* assembly, Part* part);
void assembly_add_coincident_constraint(Assembly* assembly, Part* part1, Part* part2);
void assembly_add_distance_constraint(Assembly* assembly, Part* part1, Part* part2, float distance);
void assembly_add_parallel_constraint(Assembly* assembly, Part* part1, Part* part2);
void assembly_add_perpendicular_constraint(Assembly* assembly, Part* part1, Part* part2);
bool assembly_solve_constraints(Assembly* assembly);
bool assembly_detect_collisions(Assembly* assembly);
bool assembly_save(Assembly* assembly, const char* filename);
Assembly* assembly_load(const char* filename);
bool assembly_export_stl(Assembly* assembly, const char* filename);
bool assembly_export_step(Assembly* assembly, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // CAD_PART_API_H
