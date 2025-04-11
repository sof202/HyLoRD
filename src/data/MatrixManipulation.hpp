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

}  // namespace Hylord::MatrixManipulation

#endif
