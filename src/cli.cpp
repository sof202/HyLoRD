#include "cli.hpp"

#include <limits>
#include <string>
#include <thread>

#include "CLI/CLI.hpp"

namespace Hylord::CMD {
void setup_cli(CLI::App& app, HylordConfig& config) {
   app.description(
       "HyLoRD, A hybrid cell type deconvolution algorithm for long read "
       "(ONT) data.");

   app.add_option("-t,--threads",
                  config.num_threads,
                  "Number of threads to use (range [0-total_threads]). [0]")
       ->check(CLI::Range(
           0, static_cast<int>(std::thread::hardware_concurrency())));

   app.add_option("-c,--cpg-list",
                  config.cpg_list_file,
                  "List of CpG sites (BED4 format) to use with "
                  "deconvolution algorithm (read docs for more info). "
                  "Defaults to all CpG sites in bedmethyl file.")
       ->check(CLI::ExistingFile);

   app.add_option("-r,--reference-matrix",
                  config.reference_matrix_file,
                  "Matrix of reference methylation signals (BED4+x) where x "
                  "is the number of cell types.")
       ->check(CLI::ExistingFile);

   app.add_option("-l,--cell-type-list",
                  config.cell_type_list_file,
                  "If a reference matrix is given, one can provide a list of "
                  "cell types (newline separated) corresponding with each "
                  "column of the matrix. If not provided labels will be "
                  "generic (cell_type_1, cell_type_2, etc.).")
       ->check(CLI::ExistingFile);

   app.add_option(
          "--additional-cell-types",
          config.additional_cell_types,
          "The number of expected additional cell types (range [0-100]). Read "
          "docs for additional information. [0]")
       ->check(CLI::Range(0, 100));

   app.add_option(
       "-o,--outdir",
       config.out_file_path,
       "A file path to write the determined cell proportions to (e.g. "
       ".../proportions.txt). By default, this information is written to "
       "stdout.");

   app.add_option(
          "--max-iterations",
          config.max_iterations,
          "The maximum number of iterations of main deconvolution loop (range "
          "[1-100]). Note: "
          "This does nothing if additional-cell-types is not set. [5]")
       ->check(CLI::Range(1, 100));

   app.add_option("--loop-tolerance",
                  config.loop_tolerance,
                  "The criterion for executing another iteration of the main "
                  "deconvolution loop (min: 0). Note: This does nothing if "
                  "additional-cell-types is not set. [1e-8]")
       ->check(CLI::Range(0.0, std::numeric_limits<double>::max()));

   app.add_option("bedMethyl",
                  config.bedmethyl_file,
                  "The bedMethyl file for your long read dataset obtained "
                  "from modkit (BED9+9).")
       ->required()
       ->check(CLI::ExistingFile);
}

}  // namespace Hylord::CMD
