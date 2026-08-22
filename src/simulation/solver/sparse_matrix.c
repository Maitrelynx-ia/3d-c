#include "sparse_matrix.h"
#include <stdlib.h>
#include <string.h>

SparseMatrix* sparse_matrix_create(int n) {
    SparseMatrix* mat = (SparseMatrix*)malloc(sizeof(SparseMatrix));
    if (!mat) return NULL;
    mat->n = n; mat->nnz = 0; mat->values = NULL; mat->col_indices = NULL;
    mat->row_ptr = (int*)calloc(n + 1, sizeof(int));
    if (!mat->row_ptr) { free(mat); return NULL; }
    return mat;
}

void sparse_matrix_destroy(SparseMatrix* mat) {
    if (!mat) return; if (mat->values) free(mat->values); if (mat->col_indices) free(mat->col_indices); if (mat->row_ptr) free(mat->row_ptr); free(mat);
}

void sparse_matrix_set(SparseMatrix* mat, int row, int col, float value) {
    if (!mat || row < 0 || row >= mat->n || col < 0 || col >= mat->n) return;
    int new_nnz = mat->nnz + 1;
    float* new_values = (float*)realloc(mat->values, new_nnz * sizeof(float));
    int* new_col_indices = (int*)realloc(mat->col_indices, new_nnz * sizeof(int));
    if (!new_values || !new_col_indices) return;
    mat->values = new_values; mat->col_indices = new_col_indices;
    mat->values[mat->nnz] = value; mat->col_indices[mat->nnz] = col; mat->nnz++;
}

float sparse_matrix_get(const SparseMatrix* mat, int row, int col) {
    if (!mat || row < 0 || row >= mat->n || col < 0 || col >= mat->n) return 0.0f;
    for (int i = 0; i < mat->nnz; i++) if (mat->col_indices[i] == col) return mat->values[i];
    return 0.0f;
}

Vector* vector_create(int n) {
    Vector* vec = (Vector*)malloc(sizeof(Vector));
    if (!vec) return NULL;
    vec->n = n; vec->data = (float*)calloc(n, sizeof(float));
    if (!vec->data) { free(vec); return NULL; }
    return vec;
}

void vector_destroy(Vector* vec) { if (!vec) return; if (vec->data) free(vec->data); free(vec); }
void vector_set(Vector* vec, int index, float value) { if (!vec || index < 0 || index >= vec->n) return; vec->data[index] = value; }
float vector_get(const Vector* vec, int index) { if (!vec || index < 0 || index >= vec->n) return 0.0f; return vec->data[index]; }

Vector* sparse_matrix_multiply_vector(const SparseMatrix* mat, const Vector* vec) {
    if (!mat || !vec || mat->n != vec->n) return NULL;
    Vector* result = vector_create(mat->n); if (!result) return NULL;
    for (int i = 0; i < mat->n; i++) { float sum = 0.0f; for (int j = 0; j < mat->n; j++) sum += sparse_matrix_get(mat, i, j) * vector_get(vec, j); vector_set(result, i, sum); }
    return result;
}

Vector* vector_add(const Vector* a, const Vector* b) {
    if (!a || !b || a->n != b->n) return NULL;
    Vector* result = vector_create(a->n); if (!result) return NULL;
    for (int i = 0; i < a->n; i++) vector_set(result, i, vector_get(a, i) + vector_get(b, i));
    return result;
}

Vector* vector_subtract(const Vector* a, const Vector* b) {
    if (!a || !b || a->n != b->n) return NULL;
    Vector* result = vector_create(a->n); if (!result) return NULL;
    for (int i = 0; i < a->n; i++) vector_set(result, i, vector_get(a, i) - vector_get(b, i));
    return result;
}

Vector* vector_scale(const Vector* vec, float scalar) {
    if (!vec) return NULL;
    Vector* result = vector_create(vec->n); if (!result) return NULL;
    for (int i = 0; i < vec->n; i++) vector_set(result, i, vector_get(vec, i) * scalar);
    return result;
}

float vector_dot(const Vector* a, const Vector* b) {
    if (!a || !b || a->n != b->n) return 0.0f;
    float result = 0.0f; for (int i = 0; i < a->n; i++) result += vector_get(a, i) * vector_get(b, i);
    return result;
}

SparseMatrix* sparse_matrix_from_dense(const float* dense, int n) {
    SparseMatrix* mat = sparse_matrix_create(n); if (!mat) return NULL;
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) { float value = dense[i * n + j]; if (fabsf(value) > 1e-10f) sparse_matrix_set(mat, i, j, value); }
    return mat;
}

void sparse_matrix_to_dense(const SparseMatrix* mat, float* dense) {
    if (!mat || !dense) return; memset(dense, 0, mat->n * mat->n * sizeof(float));
}