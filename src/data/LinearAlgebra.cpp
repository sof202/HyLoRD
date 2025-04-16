/**
 * @file    LinearAlgebra.cpp
 * @brief   Implements matrix preprocessing for QPP solver inputs
 * @license MIT (See LICENSE file in the repository root)
 */

#include "data/LinearAlgebra.hpp"

#include "HylordException.hpp"
#include "types.hpp"

namespace Hylord::LinearAlgebra {
Matrix gramMatrix(const Matrix& m) {
   Matrix gram_matrix{m.transpose() * m};
   static constexpr double epsilon{1e-8};
   return gram_matrix +=
          epsilon * Matrix::Identity(gram_matrix.rows(), gram_matrix.cols());
}

Vector generateCoefficientVector(const Matrix& reference_matrix,
                                 const Vector& bulk_data) {
   // Shouldn't happen under proper usage
   if (reference_matrix.rows() != bulk_data.rows()) {
      throw DeconvolutionException(
          "Coefficient Vector Generation",
          "CpGs in bulk_data must be equal to CpGs in reference data.");
   }
   return bulk_data.transpose() * reference_matrix / 2.0;
}

double squaredDistance(const Vector& v1, const Vector& v2) {
   assert(v1.size() == v2.size() &&
          "Vectors must be of the same length to compute distance between the "
          "two of them.");
   return (v1 - v2).squaredNorm();
}

void update_reference_matrix(Eigen::Ref<Matrix> reference_matrix,
                             const Vector& cell_proportions,
                             const Vector& bulk_profile,
                             int additional_cell_types) {
   assert(additional_cell_types > 0 &&
          "Reference matrix must be extended from original.");
   const int total_cell_types{static_cast<int>(reference_matrix.cols())};
   const int k{total_cell_types - additional_cell_types};

   auto r_k = reference_matrix.leftCols(k);
   auto p_k = cell_proportions.head(k);
   const auto p_l = cell_proportions.tail(additional_cell_types);

   reference_matrix.rightCols(additional_cell_types) =
       (bulk_profile - r_k * p_k) * LinearAlgebra::pseudoInverse(p_l);
}
}  // namespace Hylord::LinearAlgebra
