#ifndef LINALG_SOLVER_H
#define LINALG_SOLVER_H

#include <cholmod.h>

#ifdef __cplusplus
extern "C" {
#endif
cholmod_dense *nnls_lawson_hanson_millipede(cholmod_sparse *A, cholmod_dense *y,
    double tolerance, int min_iterations, int max_iterations,
    int npos, int normaleq, cholmod_common *c);
cholmod_dense *nmml_poisson(cholmod_sparse *B, cholmod_sparse *extra_terms,
    cholmod_dense *data, cholmod_dense *noise, double tolerance,
    int min_iterations, int max_iterations, cholmod_common *c);
cholmod_dense *pcg_poisson(cholmod_sparse *B, cholmod_sparse *extra_terms,
    cholmod_dense *data, cholmod_dense *noise, double tolerance,
    int min_iterations, int max_iterations, cholmod_common *c);
#ifdef __cplusplus
}
#endif

#endif // LINALG_SOLVER_H

