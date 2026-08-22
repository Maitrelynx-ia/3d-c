#ifndef CAD_AABB_H
#define CAD_AABB_H

#include "vec3.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- AABB Structure ---
typedef struct {
    Vec3 min;
    Vec3 max;
} AABB;

// --- Constants ---
static const AABB AABB_EMPTY = {{INFINITY, INFINITY, INFINITY}, {-INFINITY, -INFINITY, -INFINITY}};

// --- Basic Operations ---
AABB aabb_from_point(Vec3 point);
AABB aabb_from_points(const Vec3* points, int count);
AABB aabb_from_triangles(const Vec3* vertices, int num_vertices, const unsigned int* indices, int num_indices);

// --- Merge ---
AABB aabb_merge(AABB a, AABB b);
void aabb_merge_inplace(AABB* a, AABB b);

// --- Transform ---
AABB aabb_transform(AABB a, Mat4 transform);

// --- Intersection Tests ---
bool aabb_intersect(AABB a, AABB b);
bool aabb_contains_point(AABB a, Vec3 point);
bool aabb_contains_aabb(AABB a, AABB b);
bool aabb_ray_intersect(AABB a, Vec3 ray_origin, Vec3 ray_direction, float* t_min, float* t_max);

// --- Volume & Surface Area ---
float aabb_volume(AABB a);
float aabb_surface_area(AABB a);

// --- Center & Size ---
Vec3 aabb_center(AABB a);
Vec3 aabb_size(AABB a);
Vec3 aabb_half_size(AABB a);

// --- Print ---
void aabb_print(AABB a);

#ifdef __cplusplus
}
#endif

#endif // CAD_AABB_H
