/* Harmonic-map sparse linear system: Eigen BiCGSTAB (same C API as harmonic_eigen.h). */

#include <memory>

#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "harmonic_eigen.h"

extern "C" int gmorph_harmonic_bicgstab_xy(int n, int ntrip,
                                           const int *rows, const int *cols,
                                           const double *vals,
                                           const double *bx, const double *by,
                                           double *xx, double *xy)
{
  if (n <= 0)
    return 0;
  if (ntrip < 0 || rows == NULL || cols == NULL || vals == NULL || bx == NULL
      || by == NULL || xx == NULL || xy == NULL)
    return -1;

  Eigen::SparseMatrix<double> spmat(n, n);
  if (ntrip > 0) {
    const std::unique_ptr<Eigen::Triplet<double>[]> trips(
        new Eigen::Triplet<double>[(size_t)ntrip]);
    for (int i = 0; i < ntrip; ++i)
      trips[(size_t)i] = Eigen::Triplet<double>(rows[i], cols[i], vals[i]);
    spmat.setFromTriplets(trips.get(), trips.get() + ntrip);
  }

  Eigen::Map<const Eigen::VectorXd> ebx(bx, n);
  Eigen::Map<const Eigen::VectorXd> eby(by, n);
  Eigen::VectorXd vx(n);
  Eigen::VectorXd vy(n);

  Eigen::BiCGSTAB<Eigen::SparseMatrix<double>> solver;
  solver.compute(spmat);
  if (solver.info() != Eigen::Success)
    return -2;
  vx = solver.solve(ebx);
  if (solver.info() != Eigen::Success)
    return -3;
  vy = solver.solve(eby);
  if (solver.info() != Eigen::Success)
    return -4;

  for (int i = 0; i < n; ++i) {
    xx[i] = vx[i];
    xy[i] = vy[i];
  }
  return 0;
}
