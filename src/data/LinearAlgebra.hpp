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

/**
 * @brief Eigen utilities for main HyLoRD QPP solving
 */
namespace Hylord::LinearAlgebra {

/**
 * @brief Computes Gram matrix (AᵀA) which is positive semi-definite
 * @param m Input matrix (rows = features, cols = samples)
 * @return Positive semi-definite matrix of size cols×cols
 *
 * @note The expected matrix input has a huge number of rows and so it is
 * expected that the columns are linearly independent (unless user inputs data
 * with duplicated columns). As such this should be positive definite.
 */
auto gramMatrix(const Matrix& matrix) -> Matrix;

/**
 * @brief Generates coefficient vector for QPP solver
 * @param reference_matrix Reference data matrix
 * @param bulk_data Observation vector from bedmethyl file
 * @return Coefficient vector for quadratic program
 * @throws DeconvolutionException If dimensions don't match
 */
auto generateCoefficientVector(const Matrix& reference_matrix,
                               const Vector& bulk_data) -> Vector;

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
 * @brief Computes the squared distance between two dynamic vectors
 */
auto squaredDistance(const Vector& vec1, const Vector& vec2) -> double;

/**
 * @brief Update process for unknown reference profiles (see docs for more
 * info).
 */
void update_reference_matrix(Eigen::Ref<Matrix> reference_matrix,
                             const Vector& cell_proportions,
                             const Vector& bulk_profile,
                             int additional_cell_types);

}  // namespace Hylord::LinearAlgebra

#endif
