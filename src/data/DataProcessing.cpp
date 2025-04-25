/**
 * @file    DataProcessing.cpp
 * @brief   Defines functions to load and preprocess input files for HyLoRD.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/DataProcessing.hpp"

#include <utility>

#include "HylordException.hpp"
#include "data/BedData.hpp"

namespace Hylord::Processing {
/**
 * Processes input data, ensuring row consistency between bedmethyl data and
 * reference matrix. Optionally subsets both datasets based on a CpG list and
 * adds specified additional cell types if given by user.
 *
 * @throws PreprocessingException if subsetting fails or no overlapping indexes
 * are found.
 */
void preprocessInputData(BedData::BedMethylData& bedmethyl,
                         BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types) {
   if (reference_matrix.empty())
      reference_matrix = BedData::ReferenceMatrixData{bedmethyl};
   if (!cpg_list.empty()) {
      try {
         reference_matrix.subsetRows(BedData::findIndexesInCpGList(
             cpg_list, reference_matrix.records()));
      } catch (const std::exception& e) {
         throw PreprocessingException("Subset Reference Matrix on CpG List",
                                      e.what());
      }
      try {
         bedmethyl.subsetRows(
             BedData::findIndexesInCpGList(cpg_list, bedmethyl.records()));
      } catch (const std::exception& e) {
         throw PreprocessingException("Subset Bedmethyl File on CpG List",
                                      e.what());
      }
   }
   std::pair<RowIndexes, RowIndexes> overlapping_indexes{
       BedData::findOverLappingIndexes(reference_matrix.records(),
                                       bedmethyl.records())};

   if (overlapping_indexes.first.empty() ||
       overlapping_indexes.second.empty()) {
      throw PreprocessingException(
          "Find Overlapping Indexes",
          "No overlapping indexes found between reference matrix and input "
          "bedmethyl file.");
   }
   reference_matrix.subsetRows(overlapping_indexes.first);
   bedmethyl.subsetRows(overlapping_indexes.second);

   reference_matrix.addMoreCellTypes(additional_cell_types);
}
}  // namespace Hylord::Processing
