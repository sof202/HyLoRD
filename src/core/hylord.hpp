#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include <string_view>

#include "io/TSVFileReader.hpp"
#include "types.hpp"

namespace Hylord {

template <typename BedFile, typename BedType>
BedFile readFile(const std::string_view file_name,
                 int threads,
                 const IO::ColumnIndexes& fields_to_extract = {},
                 IO::RowFilter rowFilter = nullptr) {
   if (file_name.empty()) return BedFile{};

   return BedFile{[&]() {
      IO::TSVFileReader<BedType> reader{
          file_name, fields_to_extract, rowFilter, threads};
      reader.load();
      return reader.extractRecords();
   }()};
}
void run(const std::string_view bedmethyl_file,
         const std::string_view reference_matrix_file,
         const std::string_view cpg_list_file,
         const std::string_view cell_type_list_file,
         const int additional_cell_types,
         const int threads);

}  // namespace Hylord

#endif
