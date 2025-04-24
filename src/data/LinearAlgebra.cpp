/**
 * @file    LinearAlgebra.cpp
 * @brief   Implements matrix preprocessing for QPP solver inputs
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/LinearAlgebra.hpp"

#include "HylordException.hpp"
#include "types.hpp"

namespace Hylord::LinearAlgebra {
auto gramMatrix(const Matrix& matrix) -> Matrix {
   Matrix gram_matrix{matrix.transpose() * matrix};
   static constexpr double epsilon{1e-8};
   return gram_matrix +=
          epsilon * Matrix::Identity(gram_matrix.rows(), gram_matrix.cols());
}

auto generateCoefficientVector(const Matrix& reference_matrix,
                               const Vector& bulk_data) -> Vector {
   // Shouldn't happen under proper usage
   if (reference_matrix.rows() != bulk_data.rows()) {
      throw DeconvolutionException(
          "Coefficient Vector Generation",
          "CpGs in bulk_data must be equal to CpGs in reference data.");
   }
   return bulk_data.transpose() * reference_matrix * 0.5;
}

auto squaredDistance(const Vector& vec1, const Vector& vec2) -> double {
   assert(vec1.size() == vec2.size() &&
          "Vectors must be of the same length to compute distance between the "
          "two of them.");
   return (vec1 - vec2).squaredNorm();
}

void update_reference_matrix(Eigen::Ref<Matrix> reference_matrix,
                             const Vector& cell_proportions,
                             const Vector& bulk_profile,
                             int additional_cell_types) {
   assert(additional_cell_types > 0 &&
          "Reference matrix must be extended from original.");
   const int total_cell_types{static_cast<int>(reference_matrix.cols())};
   const int num_base_cell_types{total_cell_types - additional_cell_types};

   auto r_k = reference_matrix.leftCols(num_base_cell_types);
   auto p_k = cell_proportions.head(num_base_cell_types);
   const auto p_l = cell_proportions.tail(additional_cell_types);

   reference_matrix.rightCols(additional_cell_types) =
       (bulk_profile - r_k * p_k) * LinearAlgebra::pseudoInverse(p_l);
}
}  // namespace Hylord::LinearAlgebra
