/**
 * @file    writeMetrics.cpp
 * @brief   Defines write function for the outputs of the deconvolution
 * algorithm
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "io/writeMetrics.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "Eigen/src/Core/util/Meta.h"
#include "HylordException.hpp"
#include "cli.hpp"
#include "core/Deconvolver.hpp"
#include "data/BedRecords.hpp"
#include "io/TSVFileReader.hpp"
#include "maths/percentage.hpp"

namespace Hylord::IO {

/**
 * Creates a complete cell type name list
 *
 * 1. Reading known cell types from specified file (if provided)
 * 2. Generating default names ("unknown_cell_type_N") for any remaining types
 * 3. Ensuring output size matches deconvolution results dimension
 * The function guarantees one-to-one correspondence between:
 * - Cell type names in returned vector
 * - Proportions in deconvolver's results
 */
auto generateCellTypeList(const std::string_view cell_type_list_file,
                          const Deconvolution::Deconvolver& deconvolver)
    -> std::vector<BedRecords::CellType> {
   std::vector<BedRecords::CellType> cell_type_list{};
   if (!cell_type_list_file.empty()) {
      TSVFileReader<BedRecords::CellType> reader{cell_type_list_file};
      reader.load();
      cell_type_list = {reader.extractRecords()};
   }
   int num_remaining_cell_types{static_cast<int>(
       deconvolver.cellProportions().size() - cell_type_list.size())};
   for (int i{1}; i <= num_remaining_cell_types; ++i) {
      cell_type_list.emplace_back(
          BedRecords::CellType{"unknown_cell_type_" + std::to_string(i)});
   }
   return cell_type_list;
}

/**
 * Safely writes buffer contents to a file with comprehensive error checking.
 *
 * Handles file writing with multiple safety checks and features:
 * 1. Path validation:
 *    - Rejects directory paths
 *    - Creates parent directories if needed
 * 2. Permission verification:
 *    - Checks write permissions in target directory
 * 3. Collision handling:
 *    - Appends numbered suffixes to prevent overwriting existing files
 * 4. Atomic write operations:
 *    - Verifies successful open/write/close operations
 * 5. Error reporting:
 *    - Provides detailed error messages for all failure cases
 * @throws FileWriteException for any file system or I/O operation failure
 */
void writeToFile(const std::stringstream& buffer,
                 const std::filesystem::path& out_path) {
   if (std::filesystem::exists(out_path) &&
       std::filesystem::is_directory(out_path)) {
      throw FileWriteException(out_path.filename().string(),
                               "Path is an existing directory");
   }

   if (out_path.has_parent_path()) {
      std::filesystem::create_directories(out_path.parent_path());
   }

   auto parent_dir{out_path.has_parent_path()
                       ? out_path.parent_path()
                       : std::filesystem::current_path()};
   auto permissions{std::filesystem::status(parent_dir).permissions()};

   if ((permissions & std::filesystem::perms::owner_write) ==
           std::filesystem::perms::none &&
       (permissions & std::filesystem::perms::group_write) ==
           std::filesystem::perms::none) {
      throw FileWriteException(
          out_path.filename().string(),
          "No write permissions in directory: " + parent_dir.string());
   }

   std::filesystem::path final_path{out_path};

   // Handle case where file already exists as we don't want to overwrite any
   // files
   if (std::filesystem::exists(out_path) &&
       std::filesystem::is_regular_file(out_path)) {
      int counter{1};
      std::filesystem::path new_path{out_path};
      std::filesystem::path stem = out_path.stem();
      std::filesystem::path extension = out_path.extension();

      while (std::filesystem::exists(new_path)) {
         new_path = out_path.parent_path() /
                    (stem.string() + "_" + std::to_string(counter) +
                     extension.string());
         counter++;
      }

      std::cerr << "Warning: File " << out_path.filename().string()
                << " already exists. Writing to "
                << new_path.filename().string() << " instead.\n";
      final_path = new_path;
   }

   std::ofstream outfile(final_path, std::ios::binary);
   if (!outfile) {
      throw FileWriteException(final_path.string(),
                               "Failed to open file for writing.");
   }
   outfile << buffer.rdbuf();

   if (!outfile) {
      throw FileWriteException(final_path.string(),
                               "Failed to write to file.");
   }
   outfile.close();
   if (!outfile) {
      throw FileWriteException(final_path.string(),
                               "Failed to properly close file.");
   }
}

/**
 * Formats and outputs cell type proportions with the following logic:
 * 1. Generates cell type names from either:
 *    - Provided cell type list file, or
 *    - Default naming scheme if no file provided (or not enough cell type
 *      names given)
 * 2. Formats proportions as percentages
 * 3. Writes to either:
 *    - stdout if no output file specified, or
 *    - specified output file path
 * @throws FileWriteException if file writing fails
 */
void writeMetrics(const CMD::HylordConfig& config,
                  const Deconvolution::Deconvolver& deconvolver) {
   std::vector<BedRecords::CellType> cell_type_list{
       generateCellTypeList(config.cell_type_list_file, deconvolver)};
   assert(
       cell_type_list.size() == deconvolver.cellProportions().size() &&
       "Cell proportions vector and names of cell types must match in size.");

   std::stringstream output_buffer;
   for (std::size_t i{}; i < cell_type_list.size(); ++i) {
      output_buffer
          << cell_type_list[i].cell_type << '\t'
          << Maths::convertToPercent(
                 deconvolver.cellProportions()[static_cast<Eigen::Index>(i)])
          << '\n';
   }

   if (config.out_file_path.empty()) {
      std::cout << output_buffer.str();
   } else {
      writeToFile(output_buffer, config.out_file_path);
   }
}

}  // namespace Hylord::IO
