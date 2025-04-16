#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include <string_view>

#include "Eigen/Dense"
#include "cli.hpp"
#include "data/BedData.hpp"
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

/**
 * @brief Subsets input data to common CpGs and adds random additional cell
 * type data
 */
void preprocessInputData(const BedData::BedMethylData& bedmethyl,
                         const BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types);

int run(CMD::HylordConfig& config);

}  // namespace Hylord

#endif
