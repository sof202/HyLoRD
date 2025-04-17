#ifndef DECONVOLVER_H_
#define DECONVOLVER_H_

/**
 * @file    Deconvolver.hpp
 * @brief   Defines class for handling the deconvolution process using qpmad.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/LinearAlgebra.hpp"
#include "qpmad/solver.h"
#include "types.hpp"

namespace Hylord::Deconvolution {
class Deconvolver {
  public:
   Deconvolver(int num_cell_types, const Vector& bulk_profile) :
       m_num_cell_types(num_cell_types),
       m_bulk_profile{bulk_profile} {
      initialise();
   }

   qpmad::Solver::ReturnStatus runQpmad(const Matrix& reference);
   Vector cell_proportions() const { return m_cell_proportions; }
   double change_in_proportions() {
      return LinearAlgebra::squaredDistance(m_cell_proportions,
                                            m_prev_cell_proportions);
   }

  private:
   void initialise() {
      m_proportions_lower_bound = Vector::Zero(m_num_cell_types);
      m_proportions_upper_bound = Vector::Ones(m_num_cell_types);
      m_sum_lower_bound = Vector::Ones(1);
      m_sum_upper_bound = Vector::Ones(1);
      m_inequality_matrix = Vector::Ones(m_num_cell_types).transpose();
   }

   int m_num_cell_types;
   Vector m_cell_proportions;         // x
   Vector m_prev_cell_proportions;    // x_prev
   Vector m_proportions_lower_bound;  // lb
   Vector m_proportions_upper_bound;  // ub
   Vector m_sum_lower_bound;          // Alb
   Vector m_sum_upper_bound;          // Aub
   Matrix m_inequality_matrix;        // A
   Vector m_bulk_profile;             // h
};
}  // namespace Hylord::Deconvolution

#endif
