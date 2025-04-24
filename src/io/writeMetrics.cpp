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
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Eigen/src/Core/util/Meta.h"
#include "HylordException.hpp"
#include "cli.hpp"
#include "core/Deconvolver.hpp"
#include "io/TSVFileReader.hpp"
#include "maths/percentage.hpp"

namespace Hylord::IO {
struct CellType {
   std::string cell_type;
   static auto fromFields(const Fields& fields) -> CellType {
      if (fields[0].empty())
         throw std::runtime_error("Failed to parse fields (empty).");

      return CellType{fields[0]};
   }
};

auto generateCellTypeList(const std::string_view cell_type_list_file,
                          const Deconvolution::Deconvolver& deconvolver)
    -> std::vector<CellType> {
   std::vector<CellType> cell_type_list{};
   if (!cell_type_list_file.empty()) {
      TSVFileReader<CellType> reader{cell_type_list_file};
      reader.load();
      cell_type_list = {reader.extractRecords()};
   }
   int num_remaining_cell_types{static_cast<int>(
       deconvolver.cell_proportions().size() - cell_type_list.size())};
   for (int i{1}; i <= num_remaining_cell_types; ++i) {
      cell_type_list.emplace_back(
          CellType{"unknown_cell_type_" + std::to_string(i)});
   }
   return cell_type_list;
}

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

void writeMetrics(const CMD::HylordConfig& config,
                  const Deconvolution::Deconvolver& deconvolver) {
   std::vector<CellType> cell_type_list{
       generateCellTypeList(config.cell_type_list_file, deconvolver)};
   assert(
       cell_type_list.size() == deconvolver.cell_proportions().size() &&
       "Cell proportions vector and names of cell types must match in size.");

   std::stringstream output_buffer;
   for (std::size_t i{}; i < cell_type_list.size(); ++i) {
      output_buffer
          << cell_type_list[i].cell_type << '\t'
          << Maths::convertToPercent(
                 deconvolver.cell_proportions()[static_cast<Eigen::Index>(i)])
          << '\n';
   }

   if (config.out_file_path.empty()) {
      std::cout << output_buffer.str();
   } else {
      writeToFile(output_buffer, config.out_file_path);
   }
}

}  // namespace Hylord::IO
