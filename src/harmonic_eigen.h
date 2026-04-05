/* Sparse BiCGSTAB solver API for harmonic maps (implemented in harmonic_eigen.cpp). */

#ifndef GMORPH_HARMONIC_EIGEN_H
#define GMORPH_HARMONIC_EIGEN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Solve spmat * xx = bx and spmat * xy = by with BiCGSTAB.
 * @param n      matrix dimension (hgvn)
 * @param ntrip  number of triplets
 * @param rows, cols, vals  COO entries (0-based indices)
 * @return 0 on success, negative on error
 */
int gmorph_harmonic_bicgstab_xy(int n, int ntrip,
                                const int *rows, const int *cols, const double *vals,
                                const double *bx, const double *by,
                                double *xx, double *xy);

#ifdef __cplusplus
}
#endif

#endif
