#ifndef MATRIX_MANIPULATION_H_
#define MATRIX_MANIPULATION_H_

/**
 * @file    MatrixManipulation.hpp
 * @brief   Defines matrix preprocessing for QPP solver inputs
 * @license MIT (See LICENSE file in the repository root)
 */

#include "types.hpp"

/**
 * @brief Matrix manipulation utilities for QPP solver input preparation
 */
namespace Hylord::MatrixManipulation {

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
 * @throws std::invalid_argument If dimensions don't match
 */
Vector generateCoefficientVector(const Matrix& reference_matrix,
                                 const Vector& bulk_data);

template <typename Derived>
Eigen::RowVectorXd pseudoInverse(const Eigen::MatrixBase<Derived>& v) {
   static_assert(Derived::ColsAtCompileTime == 1,
                 "Input must be a column vector");
   const double squared_norm = v.squaredNorm();
   if (squared_norm < 1e-10) {
      throw std::runtime_error(
          "Norm of vector is too small for numerical stability.");
   }
   return v.transpose() / squared_norm;
}
}  // namespace Hylord::MatrixManipulation

#endif
