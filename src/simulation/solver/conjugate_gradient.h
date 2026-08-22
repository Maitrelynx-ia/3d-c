#ifndef CAD_CONJUGATE_GRADIENT_H
#define CAD_CONJUGATE_GRADIENT_H

#include "sparse_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

Vector* conjugate_gradient(const SparseMatrix* A, const Vector* b, float tol, int max_iter);

#ifdef __cplusplus
}
#endif

#endif // CAD_CONJUGATE_GRADIENT_H
