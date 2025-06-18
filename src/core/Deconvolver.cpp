/**
 * @file    Deconvolver.cpp
 * @brief   Defines main run method for Deconvolver class.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "core/Deconvolver.hpp"

#include "qpmad/solver.h"
#include "types.hpp"

namespace Hylord::Deconvolution {
/**
 * Solves the quadratic programming problem using qpmad for cell proportion
 * estimation. Uses reference matrix to construct Hessian and linear terms for
 * optimization. Applies bounds and constraints on cell proportions during
 * optimization. Stores solution in m_cell_proportions.
 */
auto Deconvolver::runQpmad(const Matrix& reference_matrix)
    -> qpmad::Solver::ReturnStatus {
   Matrix hessian{LinearAlgebra::gramMatrix(reference_matrix)};
   Vector linear_terms{LinearAlgebra::generateCoefficientVector(
       reference_matrix, m_bulk_profile)};

   qpmad::Solver qpp_solver;
   m_prev_cell_proportions.noalias() = m_cell_proportions;
   return qpp_solver.solve(m_cell_proportions,
                           hessian,
                           linear_terms,
                           m_proportions_lower_bound,
                           m_proportions_upper_bound,
                           m_inequality_matrix,
                           m_sum_lower_bound,
                           m_sum_upper_bound);
}

auto Deconvolver::evaluateObjectiveFunction(const Matrix& reference)
    -> double {
   Vector difference_vector{m_bulk_profile.transpose() -
                            (reference * m_cell_proportions)};
   return difference_vector.norm();
}
}  // namespace Hylord::Deconvolution
