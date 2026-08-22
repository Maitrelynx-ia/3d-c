#include "conjugate_gradient.h"
#include "sparse_matrix.h"
#include <math.h>

Vector* conjugate_gradient(const SparseMatrix* A, const Vector* b, float tol, int max_iter) {
    if (!A || !b || A->n != b->n) return NULL;
    int n = A->n;
    Vector* x = vector_create(n); Vector* r = vector_create(n); Vector* p = vector_create(n); Vector* Ap = vector_create(n);
    if (!x || !r || !p || !Ap) { vector_destroy(x); vector_destroy(r); vector_destroy(p); vector_destroy(Ap); return NULL; }
    for (int i = 0; i < n; i++) vector_set(x, i, 0.0f);
    Vector* temp = sparse_matrix_multiply_vector(A, x);
    for (int i = 0; i < n; i++) vector_set(r, i, vector_get(b, i) - vector_get(temp, i));
    vector_destroy(temp);
    for (int i = 0; i < n; i++) vector_set(p, i, vector_get(r, i));
    float rsold = vector_dot(r, r);
    for (int iter = 0; iter < max_iter; iter++) {
        temp = sparse_matrix_multiply_vector(A, p);
        if (!temp) { vector_destroy(x); vector_destroy(r); vector_destroy(p); vector_destroy(Ap); return NULL; }
        for (int i = 0; i < n; i++) vector_set(Ap, i, vector_get(temp, i));
        vector_destroy(temp);
        float pAp = vector_dot(p, Ap);
        if (fabsf(pAp) < 1e-10f) break;
        float alpha = rsold / pAp;
        temp = vector_scale(p, alpha); Vector* x_new = vector_add(x, temp); vector_destroy(temp); vector_destroy(x); x = x_new;
        temp = vector_scale(Ap, alpha); Vector* r_new = vector_subtract(r, temp); vector_destroy(temp); vector_destroy(r); r = r_new;
        float rsnew = vector_dot(r, r);
        if (sqrtf(rsnew) < tol) break;
        float beta = rsnew / rsold;
        temp = vector_scale(p, beta); Vector* p_new = vector_add(r, temp); vector_destroy(temp); vector_destroy(p); p = p_new;
        rsold = rsnew;
    }
    vector_destroy(r); vector_destroy(p); vector_destroy(Ap);
    return x;
}