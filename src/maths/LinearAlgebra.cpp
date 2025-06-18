/**
 * @file    LinearAlgebra.cpp
 * @brief   Implements matrix preprocessing for QPP solver inputs
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "maths/LinearAlgebra.hpp"

#include "HylordException.hpp"
#include "types.hpp"

namespace Hylord::LinearAlgebra {
/**
 * Calculates the Gram matrix (X^T * X) and adds a small diagonal
 * regularization term. The regularization term (ε*I) helps ensure numerical
 * stability with ε = 1e-8. The input matrix must have compatible dimensions
 * for matrix multiplication.
 */
auto gramMatrix(const Matrix& matrix) -> Matrix {
   Matrix gram_matrix{matrix.transpose() * matrix};
   static constexpr double epsilon{1e-8};
   return gram_matrix +=
          epsilon * Matrix::Identity(gram_matrix.rows(), gram_matrix.cols());
}

/**
 * Computes coefficient vector -(bulk^T * reference) for deconvolution.
 * Requires matching dimensions between reference matrix rows and bulk data
 * size.
 * @throws DeconvolutionException if row dimensions don't match
 */
auto generateCoefficientVector(const Matrix& reference_matrix,
                               const Vector& bulk_data) -> Vector {
   // Shouldn't happen under proper usage
   if (reference_matrix.rows() != bulk_data.rows()) {
      throw DeconvolutionException(
          "Coefficient Vector Generation",
          "CpGs in bulk_data must be equal to CpGs in reference data.");
   }
   return -(bulk_data.transpose() * reference_matrix);
}

/**
 * Extends the reference matrix by solving for additional cell type profiles
 * using bulk data. Requires the reference matrix to have space allocated for
 * additional cell types. Uses pseudoinverse to solve for new profiles based on
 * residual bulk signal. See @ref reference-matrix-updating for a mathematical
 * explanation of this.
 * @throws std::invalid_argument if additional_cell_types is not positive
 */
void updateReferenceMatrix(Eigen::Ref<Matrix> reference_matrix,
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
