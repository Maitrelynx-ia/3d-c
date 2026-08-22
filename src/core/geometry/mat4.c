#include "mat4.h"
#include <stdio.h>
#include <math.h>

Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i * 4 + j] += a.m[k * 4 + j] * b.m[i * 4 + k];
            }
        }
    }
    return result;
}

Vec3 mat4_mul_vec3(Mat4 m, Vec3 v) {
    return (Vec3){
        m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z,
        m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z,
        m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z
    };
}

Vec3 mat4_mul_vec3_projective(Mat4 m, Vec3 v) {
    float x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12];
    float y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13];
    float z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14];
    float w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15];
    if (w != 0.0f) {
        return (Vec3){x / w, y / w, z / w};
    }
    return (Vec3){x, y, z};
}

Mat4 mat4_translation(Vec3 translation) {
    Mat4 m = MAT4_IDENTITY;
    m.m[12] = translation.x;
    m.m[13] = translation.y;
    m.m[14] = translation.z;
    return m;
}

Mat4 mat4_rotation_x(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 m = MAT4_IDENTITY;
    m.m[5] = c; m.m[6] = -s;
    m.m[9] = s; m.m[10] = c;
    return m;
}

Mat4 mat4_rotation_y(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 m = MAT4_IDENTITY;
    m.m[0] = c; m.m[8] = s;
    m.m[2] = -s; m.m[10] = c;
    return m;
}

Mat4 mat4_rotation_z(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Mat4 m = MAT4_IDENTITY;
    m.m[0] = c; m.m[1] = -s;
    m.m[4] = s; m.m[5] = c;
    return m;
}

Mat4 mat4_rotation_axis(Vec3 axis, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;
    
    Vec3 n = vec3_normalize(axis);
    
    Mat4 m;
    m.m[0] = t * n.x * n.x + c;
    m.m[1] = t * n.x * n.y - s * n.z;
    m.m[2] = t * n.x * n.z + s * n.y;
    m.m[3] = 0;
    
    m.m[4] = t * n.x * n.y + s * n.z;
    m.m[5] = t * n.y * n.y + c;
    m.m[6] = t * n.y * n.z - s * n.x;
    m.m[7] = 0;
    
    m.m[8] = t * n.x * n.z - s * n.y;
    m.m[9] = t * n.y * n.z + s * n.x;
    m.m[10] = t * n.z * n.z + c;
    m.m[11] = 0;
    
    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = 0;
    m.m[15] = 1;
    
    return m;
}

Mat4 mat4_scale(Vec3 scale) {
    Mat4 m = MAT4_IDENTITY;
    m.m[0] = scale.x;
    m.m[5] = scale.y;
    m.m[10] = scale.z;
    return m;
}

Mat4 mat4_scale_uniform(float scale) {
    return mat4_scale((Vec3){scale, scale, scale});
}

Mat4 mat4_perspective(float fov, float aspect, float near, float far) {
    float tan_half_fov = tanf(fov * 0.5f);
    Mat4 m = MAT4_ZERO;
    m.m[0] = 1.0f / (aspect * tan_half_fov);
    m.m[5] = 1.0f / tan_half_fov;
    m.m[10] = -(far + near) / (far - near);
    m.m[11] = -1.0f;
    m.m[14] = -(2.0f * far * near) / (far - near);
    return m;
}

Mat4 mat4_orthographic(float left, float right, float bottom, float top, float near, float far) {
    Mat4 m = MAT4_IDENTITY;
    m.m[0] = 2.0f / (right - left);
    m.m[5] = 2.0f / (top - bottom);
    m.m[10] = -2.0f / (far - near);
    m.m[12] = -(right + left) / (right - left);
    m.m[13] = -(top + bottom) / (top - bottom);
    m.m[14] = -(far + near) / (far - near);
    return m;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_normalize(vec3_sub(center, eye));
    Vec3 s = vec3_normalize(vec3_cross(f, vec3_normalize(up)));
    Vec3 u = vec3_cross(s, f);
    
    Mat4 m = MAT4_IDENTITY;
    m.m[0] = s.x; m.m[1] = u.x; m.m[2] = -f.x;
    m.m[4] = s.y; m.m[5] = u.y; m.m[6] = -f.y;
    m.m[8] = s.z; m.m[9] = u.z; m.m[10] = -f.z;
    m.m[12] = -vec3_dot(s, eye);
    m.m[13] = -vec3_dot(u, eye);
    m.m[14] = vec3_dot(f, eye);
    return m;
}

Mat4 mat4_transpose(Mat4 m) {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i * 4 + j] = m.m[j * 4 + i];
        }
    }
    return result;
}

Mat4 mat4_inverse(Mat4 m) {
    // Using Gaussian elimination
    Mat4 inv = MAT4_IDENTITY;
    Mat4 a = m;
    
    for (int i = 0; i < 4; i++) {
        // Find pivot
        int pivot = i;
        for (int j = i + 1; j < 4; j++) {
            if (fabsf(a.m[j * 4 + i]) > fabsf(a.m[pivot * 4 + i])) {
                pivot = j;
            }
        }
        
        // Swap rows
        if (pivot != i) {
            for (int k = 0; k < 4; k++) {
                float tmp = a.m[i * 4 + k];
                a.m[i * 4 + k] = a.m[pivot * 4 + k];
                a.m[pivot * 4 + k] = tmp;
                
                tmp = inv.m[i * 4 + k];
                inv.m[i * 4 + k] = inv.m[pivot * 4 + k];
                inv.m[pivot * 4 + k] = tmp;
            }
        }
        
        // Normalize pivot row
        float pivot_val = a.m[i * 4 + i];
        if (fabsf(pivot_val) < 1e-10f) {
            // Matrix is singular
            return MAT4_ZERO;
        }
        
        for (int k = 0; k < 4; k++) {
            a.m[i * 4 + k] /= pivot_val;
            inv.m[i * 4 + k] /= pivot_val;
        }
        
        // Eliminate other rows
        for (int j = 0; j < 4; j++) {
            if (j != i) {
                float factor = a.m[j * 4 + i];
                for (int k = 0; k < 4; k++) {
                    a.m[j * 4 + k] -= a.m[i * 4 + k] * factor;
                    inv.m[j * 4 + k] -= inv.m[i * 4 + k] * factor;
                }
            }
        }
    }
    
    return inv;
}

float mat4_determinant(Mat4 m) {
    // Using Laplace expansion (simplified for 4x4)
    float det = 0;
    float sign = 1;
    
    for (int i = 0; i < 4; i++) {
        // Create 3x3 minor
        float minor[3][3];
        int minor_row = 0;
        for (int j = 1; j < 4; j++) {
            int minor_col = 0;
            for (int k = 0; k < 4; k++) {
                if (k == i) continue;
                minor[minor_row][minor_col++] = m.m[j * 4 + k];
            }
            minor_row++;
        }
        
        // Calculate 3x3 determinant
        float minor_det = minor[0][0] * (minor[1][1] * minor[2][2] - minor[1][2] * minor[2][1]) -
                          minor[0][1] * (minor[1][0] * minor[2][2] - minor[1][2] * minor[2][0]) +
                          minor[0][2] * (minor[1][0] * minor[2][1] - minor[1][1] * minor[2][0]);
        
        det += sign * m.m[i] * minor_det;
        sign = -sign;
    }
    
    return det;
}

void mat4_print(Mat4 m) {
    printf("Mat4:\n");
    for (int i = 0; i < 4; i++) {
        printf("  [%.4f, %.4f, %.4f, %.4f]\n", 
               m.m[i], m.m[i + 4], m.m[i + 8], m.m[i + 12]);
    }
}
