#include "core/hylord.hpp"

#include <cassert>
#include <exception>
#include <iostream>

#include "Eigen/Dense"
#include "cli.hpp"
#include "core/Deconvolver.hpp"
#include "data/BedData.hpp"
#include "data/BedRecords.hpp"
#include "data/DataProcessing.hpp"
#include "data/LinearAlgebra.hpp"
#include "io/writeMetrics.hpp"
#include "types.hpp"

namespace Hylord {

int run(CMD::HylordConfig& config) {
   try {
      // --------------- //
      // Data processing //
      // --------------- //
      BedData::CpGData cpg_list{
          Processing::readFile<BedData::CpGData, BedRecords::Bed4>(
              config.cpg_list_file, config.num_threads)};

      BedData::ReferenceMatrixData reference_matrix_data{
          Processing::readFile<BedData::ReferenceMatrixData,
                               BedRecords::Bed4PlusX>(
              config.reference_matrix_file, config.num_threads)};

      // chr, start, end, name and fraction modified (see Modkit README)
      IO::ColumnIndexes bedmethyl_important_fields{0, 1, 2, 3, 10};
      BedData::BedMethylData bedmethyl{
          Processing::readFile<BedData::BedMethylData, BedRecords::Bed9Plus9>(
              config.bedmethyl_file,
              config.num_threads,
              bedmethyl_important_fields)};

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
         return 0;
      }

      for (int iter{}; iter < config.max_iterations; ++iter) {
         deconvolver.runQpmad(reference_matrix);
         try {
            LinearAlgebra::update_reference_matrix(
                reference_matrix,
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
