#include "core/hylord.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include "types.hpp"

namespace Hylord {
int run(const std::string_view bedmethyl_file,
        const std::string_view reference_matrix_file,
        const std::string_view cpg_list_file,
        const std::string_view cell_type_list_file,
        const int additional_cell_types,
        const int threads) {
   try {
      BedData::CpGData cpg_list{readFile<BedData::CpGData, BedRecords::Bed4>(
          cpg_list_file, threads)};

      BedData::ReferenceMatrixData reference_matrix{
          readFile<BedData::ReferenceMatrixData, BedRecords::Bed4PlusX>(
              reference_matrix_file, threads)};

      // chr, start, end, name and fraction modified (see Modkit README)
      IO::ColumnIndexes bedmethyl_important_fields{0, 1, 2, 3, 10};
      BedData::BedMethylData bedmethyl{
          readFile<BedData::BedMethylData, BedRecords::Bed9Plus9>(
              bedmethyl_file, threads, bedmethyl_important_fields)};

      preprocessInputData(
          bedmethyl, reference_matrix, cpg_list, additional_cell_types);

      return 0;
   } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << '\n';
      return 1;
   }
}
}  // namespace Hylord
