#include "data/MatrixManipulation.hpp"

#include <stdexcept>

#include "types.hpp"

namespace Hylord::MatrixManipulation {
// Guarantees property of positive semi-definite. If the columns are linearly
// independent (very likely when number of rows is high and number of columns
// is low), the resultant matrix is positive definite.
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
