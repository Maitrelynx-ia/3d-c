#ifndef CAD_MAT4_H
#define CAD_MAT4_H

#include "vec3.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Mat4 Structure (Column-Major) ---
typedef struct {
    float m[16]; // m[0] = (0,0), m[1] = (1,0), m[2] = (2,0), m[3] = (3,0), etc.
} Mat4;

// --- Constants ---
static const Mat4 MAT4_ZERO = {0};
static const Mat4 MAT4_IDENTITY = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

// --- Basic Operations ---
static inline Mat4 mat4_add(Mat4 a, Mat4 b) {
    Mat4 result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = a.m[i] + b.m[i];
    }
    return result;
}

static inline Mat4 mat4_sub(Mat4 a, Mat4 b) {
    Mat4 result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = a.m[i] - b.m[i];
    }
    return result;
}

static inline Mat4 mat4_mul_scalar(Mat4 a, float scalar) {
    Mat4 result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = a.m[i] * scalar;
    }
    return result;
}

// --- Matrix Multiplication ---
Mat4 mat4_mul(Mat4 a, Mat4 b);

// --- Vector Transformation ---
Vec3 mat4_mul_vec3(Mat4 m, Vec3 v);
Vec3 mat4_mul_vec3_projective(Mat4 m, Vec3 v);

// --- Special Matrices ---
Mat4 mat4_translation(Vec3 translation);
Mat4 mat4_rotation_x(float angle);
Mat4 mat4_rotation_y(float angle);
Mat4 mat4_rotation_z(float angle);
Mat4 mat4_rotation_axis(Vec3 axis, float angle);
Mat4 mat4_scale(Vec3 scale);
Mat4 mat4_scale_uniform(float scale);
Mat4 mat4_perspective(float fov, float aspect, float near, float far);
Mat4 mat4_orthographic(float left, float right, float bottom, float top, float near, float far);
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);

// --- Inverse & Transpose ---
Mat4 mat4_transpose(Mat4 m);
Mat4 mat4_inverse(Mat4 m);
float mat4_determinant(Mat4 m);

// --- Print ---
void mat4_print(Mat4 m);

#ifdef __cplusplus
}
#endif

#endif // CAD_MAT4_H
