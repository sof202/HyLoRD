/**
 * @file    cli.cpp
 * @brief   Details the setup of the CLI11 parser
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "cli.hpp"

#include <limits>
#include <string>
#include <thread>

#include "CLI/CLI.hpp"

namespace Hylord::CMD {
/**
 * Sets up CLI11 command-line interface with all configuration options for
 * Hylord. Organizes parameters into logical groups (file paths, row filters,
 * hyperparameters). Includes validation checks and default values for all
 * optional parameters. Marks bedmethyl file path as required input.
 */
void setupCLI(CLI::App& app, HylordConfig& config) {
   app.description(
       "HyLoRD, A hybrid cell type deconvolution algorithm for long read "
       "(ONT) data.");

   app.add_option("-t,--threads",
                  config.num_threads,
                  "Number of threads to use when reading files.")
       ->capture_default_str()
       ->check(CLI::Range(
           0, static_cast<int>(std::thread::hardware_concurrency())));

   app.add_option("--additional-cell-types",
                  config.additional_cell_types,
                  "The number of expected additional cell types. YOU MUST SET "
                  "THIS IF NOT PROVIDING A REFERENCE MATRIX. "
                  "Read docs for additional information.")
       ->capture_default_str()
       ->check(CLI::Range(0, 100));

   app.add_option("--min-read-depth",
                  config.min_read_depth,
                  "Determines the minimum read depth required for a CpG site "
                  "(in the input data) "
                  "to be considered for deconvolution.")
       ->capture_default_str()
       ->group("Row filters")
       ->check(CLI::Range(0, std::numeric_limits<int>::max()));

   app.add_option("--max-read-depth",
                  config.max_read_depth,
                  "Determines the maximum read depth a CpG site (in the input "
                  "data) can have to be considered for deconvolution. "
                  "Not set by default.")
       ->group("Row filters")
       ->check(CLI::Range(0, std::numeric_limits<int>::max()));

   app.add_flag("--only-methylation-signal",
                config.use_only_methylation_signal,
                "Set this flag to only use methylation signals in "
                "deconvolution process (speed up if one of the input files "
                "has no hydroxymethylation signal).")
       ->group("Row filters");

   app.add_flag("--only-hydroxy-signal",
                config.use_only_hydroxy_signal,
                "Set this flag to only use hydroxymethylation signals in "
                "deconvolution process (useful when inspecting tissues with "
                "vastly different hydroxymethylation profiles, like brain).")
       ->group("Row filters");

   app.add_option("--max-iterations",
                  config.max_iterations,
                  "The maximum number of iterations of main deconvolution "
                  "loop. Note: Does nothing additional-cell-types is not set.")
       ->capture_default_str()
       ->group("Deconvolution hyperparameters")
       ->check(CLI::Range(1, 100));

   app.add_option("--convergence-threshold",
                  config.convergence_threshold,
                  "The criterion for breaking out of the main "
                  "deconvolution loop. i.e. if the change in cell proportions "
                  "between iterations is smaller than this threshold, the "
                  "loop breaks. Note: This does nothing if "
                  "additional-cell-types is not set.")
       ->capture_default_str()
       ->group("Deconvolution hyperparameters")
       ->check(CLI::Range(0.0, std::numeric_limits<double>::max()));

   app.add_option("-c,--cpg-list",
                  config.cpg_list_file,
                  "List of CpG sites (BED4 format) to use with "
                  "deconvolution algorithm (read docs for more info). "
                  "Defaults to all CpG sites in bedmethyl file.")
       ->group("File paths")
       ->check(CLI::ExistingFile);

   app.add_option("-r,--reference-matrix",
                  config.reference_matrix_file,
                  "Bed4+x file containing a matrix of reference methylation "
                  "signals where x is the number of cell "
                  "types.\ne.g. chr start end name cell_one cell_two...")
       ->group("File paths")
       ->check(CLI::ExistingFile);

   app.add_option("-l,--cell-type-list",
                  config.cell_type_list_file,
                  "If a reference matrix is given, one can provide a list of "
                  "cell types (newline separated) corresponding with each "
                  "column of the reference matrix (starting from 5th field).")
       ->group("File paths")
       ->check(CLI::ExistingFile);

   app.add_option("-o,--outpath",
                  config.out_file_path,
                  "A file path to write the determined cell proportions to "
                  "(e.g. .../proportions.txt). By default, this information "
                  "is written to the standard output stream.")
       ->group("File paths");

   app.add_option("bedmethyl_file_path",
                  config.bedmethyl_file,
                  "The bedMethyl file for your long read dataset obtained "
                  "from modkit (BED9+9).")
       ->required()
       ->check(CLI::ExistingFile);
}

}  // namespace Hylord::CMD
