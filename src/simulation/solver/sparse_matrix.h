#ifndef CAD_SPARSE_MATRIX_H
#define CAD_SPARSE_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int n; float* values; int* col_indices; int* row_ptr; int nnz; } SparseMatrix;
typedef struct { int n; float* data; } Vector;

SparseMatrix* sparse_matrix_create(int n);
void sparse_matrix_destroy(SparseMatrix* mat);
Vector* vector_create(int n);
void vector_destroy(Vector* vec);
void sparse_matrix_set(SparseMatrix* mat, int row, int col, float value);
float sparse_matrix_get(const SparseMatrix* mat, int row, int col);
void vector_set(Vector* vec, int index, float value);
float vector_get(const Vector* vec, int index);
Vector* sparse_matrix_multiply_vector(const SparseMatrix* mat, const Vector* vec);
Vector* vector_add(const Vector* a, const Vector* b);
Vector* vector_subtract(const Vector* a, const Vector* b);
Vector* vector_scale(const Vector* vec, float scalar);
float vector_dot(const Vector* a, const Vector* b);
SparseMatrix* sparse_matrix_from_dense(const float* dense, int n);
void sparse_matrix_to_dense(const SparseMatrix* mat, float* dense);

#ifdef __cplusplus
}
#endif

#endif // CAD_SPARSE_MATRIX_H
