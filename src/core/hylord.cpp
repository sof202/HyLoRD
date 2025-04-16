#include "core/hylord.hpp"

#include <cassert>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "Eigen/Dense"
#include "cli.hpp"
#include "core/Deconvolver.hpp"
#include "data/BedData.hpp"
#include "data/BedRecords.hpp"
#include "data/LinearAlgebra.hpp"
#include "io/writeMetrics.hpp"
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

int run(CMD::HylordConfig& config) {
   try {
      // --------------- //
      // Data processing //
      // --------------- //
      BedData::CpGData cpg_list{readFile<BedData::CpGData, BedRecords::Bed4>(
          config.cpg_list_file, config.num_threads)};

      BedData::ReferenceMatrixData reference_matrix_data{
          readFile<BedData::ReferenceMatrixData, BedRecords::Bed4PlusX>(
              config.reference_matrix_file, config.num_threads)};

      // chr, start, end, name and fraction modified (see Modkit README)
      IO::ColumnIndexes bedmethyl_important_fields{0, 1, 2, 3, 10};
      BedData::BedMethylData bedmethyl{
          readFile<BedData::BedMethylData, BedRecords::Bed9Plus9>(
              config.bedmethyl_file,
              config.num_threads,
              bedmethyl_important_fields)};

      preprocessInputData(bedmethyl,
                          reference_matrix_data,
                          cpg_list,
                          config.additional_cell_types);
      Vector bulk_profile{bedmethyl.getAsEigenVector()};
      Matrix reference_matrix{reference_matrix_data.getAsEigenMatrix()};

      // ------------- //
      // Deconvolution //
      // ------------- //
      Deconvolution::Deconvolver deconvolver{
          reference_matrix_data.numberOfCellTypes(), bulk_profile};
      if (config.additional_cell_types == 0) {
         deconvolver.runQpmad(reference_matrix);
         return 0;
      }

      for (int iter{}; iter < config.max_iterations; ++iter) {
         deconvolver.runQpmad(reference_matrix);
         try {
            update_reference_matrix(reference_matrix,
                                    deconvolver.cell_proportions(),
                                    bulk_profile,
                                    config.additional_cell_types);
         } catch (const std::exception& e) {
            std::cerr
                << "Error: " << e.what() << " (iteration: " << iter << ')'
                << "\n Rerunning HyLoRD with a lower number of iterations "
                   "(--max-iterations) might help. If not, please "
                   "consult the documentation.\n";
            break;
         }
         // On first iteration, there is no 'previous' cell proportions
         // yet and so the distance metric will fail.
         if (iter > 0 && deconvolver.change_in_proportions() <
                             config.convergence_threshold) {
            std::cout << "Deconvolution loop finished after " << iter
                      << " iterations.\n";
            break;
         }
      }

      // ------- //
      // Outputs //
      // ------- //
      IO::writeMetrics(config, deconvolver);

      return 0;
   } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << '\n';
      return 1;
   }
}
}  // namespace Hylord
