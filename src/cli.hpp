#ifndef HYLORD_CLI_H_
#define HYLORD_CLI_H_

/**
 * @file    cli.hpp
 * @brief   Declares cli setup and config options struct
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <limits>

#include "CLI/App.hpp"

namespace Hylord::CMD {
struct HylordConfig {
   int num_threads = 0;
   std::string cpg_list_file;
   std::string reference_matrix_file;
   std::string cell_type_list_file;
   int additional_cell_types = 0;
   std::string out_file_path;
   int max_iterations = 5;
   double convergence_threshold = 1e-8;
   std::string bedmethyl_file;
   int min_read_depth = 10;
   int max_read_depth = std::numeric_limits<int>::max();
   bool use_only_methylation_signal = false;
   bool use_only_hydroxy_signal = false;
};
void setupCLI(CLI::App& app, HylordConfig& config);
}  // namespace Hylord::CMD

#endif
