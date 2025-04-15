#include <string>
#include <thread>

#include "CLI/CLI.hpp"
#include "core/hylord.hpp"

int main(int argc, char** argv) {
   try {
      CLI::App application{
          "HyLoRD, A hybrid cell type deconvolution algorithm for long read "
          "(ONT) data."};

      int num_threads{};
      application.add_option(
          "-t,--threads", num_threads, "Number of threads to use [0].");

      std::string cpg_list_file{};
      application
          .add_option("-c,--cpg-list",
                      cpg_list_file,
                      "List of CpG sites (BED4 format) to use with "
                      "deconvolution algorithm (read docs for more info). "
                      "Defaults to all CpG sites in bedmethyl file.")
          ->check(CLI::ExistingFile);

      std::string reference_matrix_file{};
      application
          .add_option(
              "-r,--reference-matrix",
              reference_matrix_file,
              "Matrix of reference methylation signals (BED4+x) where x "
              "is the number of cell types.")
          ->check(CLI::ExistingFile);

      std::string cell_type_list_file{};
      application
          .add_option(
              "-l,--cell-type-list",
              cell_type_list_file,
              "If a reference matrix is given, one can provide a list of "
              "cell types (newline separated) corresponding with each "
              "column of the matrix. If not provided labels will be "
              "generic (cell_type_1, cell_type_2, etc.).")
          ->check(CLI::ExistingFile);

      int additional_cell_types{};
      application.add_option(
          "--additional-cell-types",
          additional_cell_types,
          "The number of expected additional cell types. Read "
          "docs for additional information.");

      std::string bedmethyl_file{};
      application
          .add_option("bedMethyl",
                      bedmethyl_file,
                      "The bedMethyl file for your long read dataset obtained "
                      "from modkit (BED9+9).")
          ->required()
          ->check(CLI::ExistingFile);

      CLI11_PARSE(application, argc, argv);

      if (num_threads == 0)
         num_threads = static_cast<int>(std::thread::hardware_concurrency());

      Hylord::run(bedmethyl_file,
                  reference_matrix_file,
                  cpg_list_file,
                  cell_type_list_file,
                  additional_cell_types,
                  num_threads);
      return 0;
   } catch (...) {
      std::cerr << "An unexpected fatal error occurred.\n";
      return 1;
   }
}
