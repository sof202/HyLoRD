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

/// Defines methods for processing ready for main deconvolution loop
namespace Hylord::Processing {
/**
 * Reads and parses a BED file into a specified container type with optional
 * filtering.
 *
 * Reads a BED-formatted file using multiple threads if specified,
 * with options for column selection and row filtering. Returns an empty
 * container if the filename is empty. Threads and field selection can be
 * customized.
 */
template <typename BedFile, typename BedType>
auto readFile(const std::string_view file_name,
              int threads,
              const IO::ColumnIndexes& fields_to_extract = {},
              IO::RowFilter rowFilter = nullptr) -> BedFile {
   if (file_name.empty()) return BedFile{};

   return BedFile{[&]() {
      IO::TSVFileReader<BedType> reader{
          file_name, fields_to_extract, rowFilter, threads};
      reader.load();
      return reader.extractRecords();
   }()};
}

/// Preprocesses input data by aligning and subsetting bedmethyl and reference
/// matrix data.
void preprocessInputData(BedData::BedMethylData& bedmethyl,
                         BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types);

}  // namespace Hylord::Processing

#endif
