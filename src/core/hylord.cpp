/**
 * @file    hylord.cpp
 * @brief   Defines the main Hylord run function.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "core/hylord.hpp"

#include <cassert>
#include <exception>
#include <iostream>

#include "Eigen/Dense"
#include "HylordException.hpp"
#include "cli.hpp"
#include "core/Deconvolver.hpp"
#include "data/BedData.hpp"
#include "data/BedRecords.hpp"
#include "data/DataProcessing.hpp"
#include "data/Filters.hpp"
#include "io/writeMetrics.hpp"
#include "maths/LinearAlgebra.hpp"
#include "types.hpp"

namespace Hylord {

/**
 * Main workflow function that performs:
 * 1. Data processing:
 *    - Reads and filters CpG list, reference matrix, and bedmethyl data
 *    - Preprocesses input data into numerical matrices
 * 2. Deconvolution:
 *    - Initializes deconvolution solver
 *    - Runs iterative deconvolution with reference matrix updates
 * 3. Output:
 *    - Writes final metrics and proportions (possibly to a file)
 */
auto run(CMD::HylordConfig& config) -> int {
   try {
      // --------------- //
      // Data processing //
      // --------------- //

      if (config.reference_matrix_file.empty() &&
          config.additional_cell_types == 0) {
         throw HylordException(
             "If no reference matrix is provided, additional_cell_types "
             "should be set (>0).");
      }

      IO::RowFilter mark_filter{Filters::generateNameFilter(config)};
      BedData::CpGData cpg_list{
          Processing::readFile<BedData::CpGData, BedRecords::Bed4>(
              config.cpg_list_file, config.num_threads, {}, mark_filter)};

      BedData::ReferenceMatrixData reference_matrix_data{
          Processing::readFile<BedData::ReferenceMatrixData,
                               BedRecords::Bed4PlusX>(
              config.reference_matrix_file,
              config.num_threads,
              {},
              mark_filter)};

      // chr, start, end, name, score (read_depth) and fraction modified (see
      // Modkit README)
      IO::ColumnIndexes bedmethyl_important_fields{0, 1, 2, 3, 4, 10};
      IO::RowFilter bedmethyl_row_filter{
          Filters::generateBedmethylRowFilter(config)};
      BedData::BedMethylData bedmethyl{
          Processing::readFile<BedData::BedMethylData, BedRecords::Bed9Plus9>(
              config.bedmethyl_file,
              config.num_threads,
              bedmethyl_important_fields,
              bedmethyl_row_filter)};

      Processing::preprocessInputData(bedmethyl,
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
         IO::writeMetrics(config, deconvolver);
         return 0;
      }

      int iteration{1};
      while (iteration <= config.max_iterations) {
         deconvolver.runQpmad(reference_matrix);
         try {
            LinearAlgebra::updateReferenceMatrix(reference_matrix,
                                                 deconvolver.cellProportions(),
                                                 bulk_profile,
                                                 config.additional_cell_types);
         } catch (const std::exception& e) {
            std::cerr << "Warning: " << e.what()
                      << " Reference matrix could not be updated as a result "
                         "(iteration: "
                      << iteration << ").\n"
                      << "Rerunning HyLoRD with a lower number of iterations "
                         "(--max-iterations) might help.\n"
                      << "If this doesn't help, please consult the "
                         "documentation or consider opening an issue at "
                         "https://github.com/sof202/HyLoRD/issues.\n";
            break;
         }
         // On first iteration, there is no 'previous' cell proportions
         // yet and so the distance metric will fail.
         if (iteration > 1 && deconvolver.changeInProportions() <
                                  config.convergence_threshold) {
            break;
         }
         iteration++;
      }
      std::cout << "Deconvolution loop finished after " << iteration
                << " iteration" << (iteration == 1 ? ".\n" : "s.\n");

      // ------- //
      // Outputs //
      // ------- //
      IO::writeMetrics(config, deconvolver);

      return 0;
   } catch (const HylordException& e) {
      std::cerr << e.what() << '\n';
      return 1;
   } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << '\n';
      return 1;
   }
}
}  // namespace Hylord
