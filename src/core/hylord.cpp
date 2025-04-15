#include "core/hylord.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "Eigen/Dense"
#include "data/BedData.hpp"
#include "data/BedRecords.hpp"
#include "data/MatrixManipulation.hpp"
#include "qpmad/solver.h"
#include "types.hpp"

namespace Hylord {
void preprocessInputData(BedData::BedMethylData& bedmethyl,
                         BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types) {
   if (reference_matrix.empty()) {
      if (additional_cell_types < 1) {
         throw std::invalid_argument(
             "If no reference matrix is provided, additional_cell_types "
             "should "
             "be set (>0).");
      }
      reference_matrix = BedData::ReferenceMatrixData{bedmethyl};
   }
   if (!cpg_list.empty()) {
      reference_matrix.subsetRows(
          BedData::findIndexesInCpGList(cpg_list, reference_matrix.records()));
      bedmethyl.subsetRows(
          BedData::findIndexesInCpGList(cpg_list, bedmethyl.records()));
   }
   std::pair<RowIndexes, RowIndexes> overlapping_indexes{
       BedData::findOverLappingIndexes(reference_matrix.records(),
                                       bedmethyl.records())};

   if (overlapping_indexes.first.empty() ||
       overlapping_indexes.second.empty()) {
      throw std::runtime_error(
          "No overlapping indexes found between reference matrix and input "
          "bedmethyl file.");
   }
   reference_matrix.subsetRows(overlapping_indexes.first);
   bedmethyl.subsetRows(overlapping_indexes.second);

   reference_matrix.addMoreCellTypes(additional_cell_types);
}

qpmad::Solver::ReturnStatus Deconvolver::runQpmad(
    const Matrix& reference_matrix) {
   Matrix Hessian{MatrixManipulation::gramMatrix(reference_matrix)};
   Vector linear_terms{MatrixManipulation::generateCoefficientVector(
       reference_matrix, m_bulk_profile)};

   qpmad::Solver qpp_solver;
   return qpp_solver.solve(m_cell_proportions,
                           Hessian,
                           linear_terms,
                           m_proportions_lower_bound,
                           m_proportions_upper_bound,
                           m_inequality_matrix,
                           m_sum_lower_bound,
                           m_sum_upper_bound);
}

/**
 * @brief Update process for unknown reference profiles (see docs for more
 * info).
 */
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
       (bulk_profile - r_k * p_k) * MatrixManipulation::pseudoInverse(p_l);
}

int run(const std::string_view bedmethyl_file,
        const std::string_view reference_matrix_file,
        const std::string_view cpg_list_file,
        const std::string_view cell_type_list_file,
        const int additional_cell_types,
        const int threads) {
   try {
      BedData::CpGData cpg_list{readFile<BedData::CpGData, BedRecords::Bed4>(
          cpg_list_file, threads)};

      BedData::ReferenceMatrixData reference_matrix_data{
          readFile<BedData::ReferenceMatrixData, BedRecords::Bed4PlusX>(
              reference_matrix_file, threads)};

      // chr, start, end, name and fraction modified (see Modkit README)
      IO::ColumnIndexes bedmethyl_important_fields{0, 1, 2, 3, 10};
      BedData::BedMethylData bedmethyl{
          readFile<BedData::BedMethylData, BedRecords::Bed9Plus9>(
              bedmethyl_file, threads, bedmethyl_important_fields)};

      preprocessInputData(
          bedmethyl, reference_matrix_data, cpg_list, additional_cell_types);

      Matrix reference_matrix{reference_matrix_data.getAsEigenMatrix()};
      Deconvolver deconvolver{reference_matrix_data.numberOfCellTypes(),
                              bedmethyl.getAsEigenVector()};
      deconvolver.runQpmad(reference_matrix);

      std::cout << reference_matrix << '\n';
      if (additional_cell_types > 0) {
         update_reference_matrix(reference_matrix,
                                 deconvolver.cell_proportions(),
                                 additional_cell_types);
      }
      std::cout << reference_matrix << '\n';
      return 0;
   } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << '\n';
      return 1;
   }
}
}  // namespace Hylord
