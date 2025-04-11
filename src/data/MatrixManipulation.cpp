/**
 * @file    MatrixManipulation.cpp
 * @brief   Implements matrix preprocessing for QPP solver inputs
 * @license MIT (See LICENSE file in the repository root)
 */

#include "data/MatrixManipulation.hpp"

#include <stdexcept>

#include "types.hpp"

namespace Hylord::MatrixManipulation {
Matrix gramMatrix(const Matrix& m) { return m.transpose() * m; }

Vector generateCoefficientVector(const Matrix& reference_matrix,
                                 const Vector& bulk_data) {
   // Shouldn't happen under proper usage
   if (reference_matrix.rows() != bulk_data.rows()) {
      throw std::invalid_argument(
          "CpGs in bulk_data must be equal to CpGs in reference data.");
   }
   return bulk_data.transpose() * reference_matrix / 2.0;
}

}  // namespace Hylord::MatrixManipulation
