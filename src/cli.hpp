#ifndef HYLORD_CLI_H_
#define HYLORD_CLI_H_

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
};
void setup_cli(CLI::App& app, HylordConfig& config);
}  // namespace Hylord::CMD

#endif
