#include "core/hylord.hpp"

#include <algorithm>
#include <exception>
#include <iomanip>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "Eigen/Dense"
#include "Eigen/src/Core/util/Constants.h"
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

int run(const std::string_view bedmethyl_file,
        const std::string_view reference_matrix_file,
        const std::string_view cpg_list_file,
        const std::string_view cell_type_list_file,
        const int additional_cell_types,
        const int threads) {
   try {
      BedData::CpGData cpg_list{readFile<BedData::CpGData, BedRecords::Bed4>(
          cpg_list_file, threads)};

      BedData::ReferenceMatrixData reference_matrix{
          readFile<BedData::ReferenceMatrixData, BedRecords::Bed4PlusX>(
              reference_matrix_file, threads)};

      // chr, start, end, name and fraction modified (see Modkit README)
      IO::ColumnIndexes bedmethyl_important_fields{0, 1, 2, 3, 10};
      BedData::BedMethylData bedmethyl{
          readFile<BedData::BedMethylData, BedRecords::Bed9Plus9>(
              bedmethyl_file, threads, bedmethyl_important_fields)};

      preprocessInputData(
          bedmethyl, reference_matrix, cpg_list, additional_cell_types);

      qpmad::Solver qpp_solver;
      int num_cell_types{reference_matrix.numberOfCellTypes()};

      // Defining qpmad solver requirements -------------------------------- //
      Vector proportions_lower_bound{Vector::Zero(num_cell_types)};  // lb
      Vector proportions_upper_bound{Vector::Ones(num_cell_types)};  // ub
      Vector sum_lower_bound{1};                                     // Alb
      Vector sum_upper_bound{1};                                     // Aub
      Vector cell_proportions;                                       // x
      Matrix Hessian{MatrixManipulation::gramMatrix(                 // H
          reference_matrix.getAsEigenMatrix())};                     // H
      Vector linear_terms{MatrixManipulation::generateCoefficientVector(   // h
          reference_matrix.getAsEigenMatrix(),                             // h
          bedmethyl.getAsEigenVector())};                                  // h
      Matrix inequality_matrix{Vector::Ones(num_cell_types).transpose()};  // A
      // ------------------------------------------------------------------- //

      qpmad::Solver::ReturnStatus status{
          qpp_solver.solve(cell_proportions,
                           Hessian,
                           linear_terms,
                           proportions_lower_bound,
                           proportions_upper_bound,
                           inequality_matrix,
                           sum_lower_bound,
                           sum_upper_bound)};

      std::cout << cell_proportions << '\n';

      return 0;
   } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << '\n';
      return 1;
   }
}
}  // namespace Hylord
