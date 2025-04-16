#include "io/writeMetrics.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cli.hpp"
#include "core/hylord.hpp"
#include "io/TSVFileReader.hpp"

namespace Hylord::IO {
std::vector<CellType> generateCellTypeList(
    const std::string_view cell_type_list_file,
    const Deconvolver& deconvolver) {
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

double convertToPercent(double d, int precision) {
   double percent{std::round(d * 100 * precision) / precision};
   return percent > 0 ? percent : 0;
}

void writeMetrics(const CMD::HylordConfig& config,
                  const Deconvolver& deconvolver) {
   std::vector<CellType> cell_type_list{
       generateCellTypeList(config.cell_type_list_file, deconvolver)};
   assert(
       cell_type_list.size() == deconvolver.cell_proportions().size() &&
       "Cell proportions vector and names of cell types must match in size.");

   std::stringstream output_buffer;
   for (std::size_t i{}; i < cell_type_list.size(); ++i) {
      output_buffer << cell_type_list[i].cell_type << '\t'
                    << convertToPercent(deconvolver.cell_proportions()[i])
                    << '\n';
   }

   if (config.out_file_path.empty()) {
      std::cout << output_buffer.str();
   } else {
      std::ofstream out_file(config.out_file_path);
      out_file << output_buffer.str();
   }
}

}  // namespace Hylord::IO
