#ifndef LINEAR_ALGEBRA_H_
#define LINEAR_ALGEBRA_H_

/**
 * @file    LinearAlgebra.hpp
 * @brief   Defines matrix preprocessing for QPP solver inputs
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <stdexcept>

#include "types.hpp"

/// Eigen utilities for main HyLoRD QPP solving
namespace Hylord::LinearAlgebra {
/// Computes the Gram matrix of the input matrix with added regularization.
auto gramMatrix(const Matrix& matrix) -> Matrix;

/// Generates coefficient vector for QPP solver
auto generateCoefficientVector(const Matrix& reference_matrix,
                               const Vector& bulk_data) -> Vector;

/**
 * Computes the pseudoinverse of a column vector.
 *
 * Calculates the pseudoinverse (v^T / (v^T v)) of a column vector for
 * least-squares solutions. Enforces compile-time check for column vector input
 * and minimum norm requirement.
 * @throws std::invalid_argument if the norm of the given vector is below the
 * stability threshold
 */
template <typename Derived>
auto pseudoInverse(const Eigen::MatrixBase<Derived>& vec)
    -> Eigen::RowVectorXd {
   static_assert(Derived::ColsAtCompileTime == 1,
                 "Input must be a column vector");
   const double squared_norm = vec.squaredNorm();
   const double min_stable_norm{1e-10};
   if (squared_norm < min_stable_norm) {
      throw std::invalid_argument(
          "Norm of vector is too small for numerical stability.");
   }
   return vec.transpose() / squared_norm;
}

/**
 * @brief Update process for unknown reference profiles (see @ref
 * reference-matrix-updating for more info).
 */
void updateReferenceMatrix(Eigen::Ref<Matrix> reference_matrix,
                           const Vector& cell_proportions,
                           const Vector& bulk_profile,
                           int additional_cell_types);

}  // namespace Hylord::LinearAlgebra

#endif
