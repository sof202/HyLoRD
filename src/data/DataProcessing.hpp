#ifndef DATA_PROCESSING_H_
#define DATA_PROCESSING_H_

/**
 * @file    DataProcessing.hpp
 * @brief   Declares functions to load and preprocess input files for HyLoRD.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/BedData.hpp"
#include "io/TSVFileReader.hpp"
#include "types.hpp"

namespace Hylord::Processing {
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
void preprocessInputData(BedData::BedMethylData& bedmethyl,
                         BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types);

}  // namespace Hylord::Processing

#endif
