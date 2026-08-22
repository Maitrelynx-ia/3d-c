#include "aabb.h"
#include "mat4.h"
#include <stdio.h>
#include <float.h>

AABB aabb_from_point(Vec3 point) {
    return (AABB){point, point};
}

AABB aabb_from_points(const Vec3* points, int count) {
    if (count == 0) return AABB_EMPTY;
    
    AABB aabb = aabb_from_point(points[0]);
    for (int i = 1; i < count; i++) {
        aabb.min.x = fminf(aabb.min.x, points[i].x);
        aabb.min.y = fminf(aabb.min.y, points[i].y);
        aabb.min.z = fminf(aabb.min.z, points[i].z);
        aabb.max.x = fmaxf(aabb.max.x, points[i].x);
        aabb.max.y = fmaxf(aabb.max.y, points[i].y);
        aabb.max.z = fmaxf(aabb.max.z, points[i].z);
    }
    return aabb;
}

AABB aabb_from_triangles(const Vec3* vertices, int num_vertices, const unsigned int* indices, int num_indices) {
    AABB aabb = AABB_EMPTY;
    
    for (int i = 0; i < num_indices; i += 3) {
        Vec3 v0 = vertices[indices[i]];
        Vec3 v1 = vertices[indices[i + 1]];
        Vec3 v2 = vertices[indices[i + 2]];
        
        aabb.min.x = fminf(fminf(aabb.min.x, v0.x), fminf(v1.x, v2.x));
        aabb.min.y = fminf(fminf(aabb.min.y, v0.y), fminf(v1.y, v2.y));
        aabb.min.z = fminf(fminf(aabb.min.z, v0.z), fminf(v1.z, v2.z));
        
        aabb.max.x = fmaxf(fmaxf(aabb.max.x, v0.x), fmaxf(v1.x, v2.x));
        aabb.max.y = fmaxf(fmaxf(aabb.max.y, v0.y), fmaxf(v1.y, v2.y));
        aabb.max.z = fmaxf(fmaxf(aabb.max.z, v0.z), fmaxf(v1.z, v2.z));
    }
    
    return aabb;
}

AABB aabb_merge(AABB a, AABB b) {
    return (AABB){
        {fminf(a.min.x, b.min.x), fminf(a.min.y, b.min.y), fminf(a.min.z, b.min.z)},
        {fmaxf(a.max.x, b.max.x), fmaxf(a.max.y, b.max.y), fmaxf(a.max.z, b.max.z)}
    };
}

void aabb_merge_inplace(AABB* a, AABB b) {
    a->min.x = fminf(a->min.x, b.min.x);
    a->min.y = fminf(a->min.y, b.min.y);
    a->min.z = fminf(a->min.z, b.min.z);
    a->max.x = fmaxf(a->max.x, b.max.x);
    a->max.y = fmaxf(a->max.y, b.max.y);
    a->max.z = fmaxf(a->max.z, b.max.z);
}

AABB aabb_transform(AABB a, Mat4 transform) {
    // Transform all 8 corners and create new AABB
    Vec3 corners[8] = {
        {a.min.x, a.min.y, a.min.z},
        {a.max.x, a.min.y, a.min.z},
        {a.min.x, a.max.y, a.min.z},
        {a.max.x, a.max.y, a.min.z},
        {a.min.x, a.min.y, a.max.z},
        {a.max.x, a.min.y, a.max.z},
        {a.min.x, a.max.y, a.max.z},
        {a.max.x, a.max.y, a.max.z}
    };
    
    for (int i = 0; i < 8; i++) {
        corners[i] = mat4_mul_vec3(transform, corners[i]);
    }
    
    return aabb_from_points(corners, 8);
}

bool aabb_intersect(AABB a, AABB b) {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z;
}

bool aabb_contains_point(AABB a, Vec3 point) {
    return point.x >= a.min.x && point.x <= a.max.x &&
           point.y >= a.min.y && point.y <= a.max.y &&
           point.z >= a.min.z && point.z <= a.max.z;
}

bool aabb_contains_aabb(AABB a, AABB b) {
    return a.min.x <= b.min.x && a.max.x >= b.max.x &&
           a.min.y <= b.min.y && a.max.y >= b.max.y &&
           a.min.z <= b.min.z && a.max.z >= b.max.z;
}

bool aabb_ray_intersect(AABB a, Vec3 ray_origin, Vec3 ray_direction, float* t_min, float* t_max) {
    float t1 = (a.min.x - ray_origin.x) / ray_direction.x;
    float t2 = (a.max.x - ray_origin.x) / ray_direction.x;
    float t3 = (a.min.y - ray_origin.y) / ray_direction.y;
    float t4 = (a.max.y - ray_origin.y) / ray_direction.y;
    float t5 = (a.min.z - ray_origin.z) / ray_direction.z;
    float t6 = (a.max.z - ray_origin.z) / ray_direction.z;
    
    float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
    float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));
    
    if (tmin > tmax) return false;
    
    if (t_min) *t_min = tmin;
    if (t_max) *t_max = tmax;
    
    return true;
}

float aabb_volume(AABB a) {
    Vec3 size = aabb_size(a);
    return size.x * size.y * size.z;
}

float aabb_surface_area(AABB a) {
    Vec3 size = aabb_size(a);
    return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
}

Vec3 aabb_center(AABB a) {
    return vec3_mul(vec3_add(a.min, a.max), 0.5f);
}

Vec3 aabb_size(AABB a) {
    return vec3_sub(a.max, a.min);
}

Vec3 aabb_half_size(AABB a) {
    return vec3_mul(aabb_size(a), 0.5f);
}

void aabb_print(AABB a) {
    printf("AABB{min=(%.2f, %.2f, %.2f), max=(%.2f, %.2f, %.2f)}",
           a.min.x, a.min.y, a.min.z, a.max.x, a.max.y, a.max.z);
}
