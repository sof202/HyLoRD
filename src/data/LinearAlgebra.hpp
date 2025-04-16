#ifndef LINEAR_ALGEBRA_H_
#define LINEAR_ALGEBRA_H_

/**
 * @file    LinearAlgebra.hpp
 * @brief   Defines matrix preprocessing for QPP solver inputs
 * @license MIT (See LICENSE file in the repository root)
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
Matrix gramMatrix(const Matrix& m);

/**
 * @brief Generates coefficient vector for QPP solver
 * @param reference_matrix Reference data matrix
 * @param bulk_data Observation vector from bedmethyl file
 * @return Coefficient vector for quadratic program
 * @throws DeconvolutionException If dimensions don't match
 */
Vector generateCoefficientVector(const Matrix& reference_matrix,
                                 const Vector& bulk_data);

template <typename Derived>
Eigen::RowVectorXd pseudoInverse(const Eigen::MatrixBase<Derived>& v) {
   static_assert(Derived::ColsAtCompileTime == 1,
                 "Input must be a column vector");
   const double squared_norm = v.squaredNorm();
   if (squared_norm < 1e-10) {
      throw std::invalid_argument(
          "Norm of vector is too small for numerical stability.");
   }
   return v.transpose() / squared_norm;
}

/**
 * @brief Computes the squared distance between two dynamic vectors
 */
double squaredDistance(const Vector& v1, const Vector& v2);

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
