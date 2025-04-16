#include "data/DataProcessing.hpp"

#include <stdexcept>
#include <utility>

#include "data/BedData.hpp"

namespace Hylord::Processing {
void preprocessInputData(BedData::BedMethylData& bedmethyl,
                         BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types) {
   if (reference_matrix.empty()) {
      if (additional_cell_types < 1) {
         throw std::invalid_argument(
             "If no reference matrix is provided, additional_cell_types "
             "should "
             "be set (>0).");
      }
      reference_matrix = BedData::ReferenceMatrixData{bedmethyl};
   }
   if (!cpg_list.empty()) {
      reference_matrix.subsetRows(
          BedData::findIndexesInCpGList(cpg_list, reference_matrix.records()));
      bedmethyl.subsetRows(
          BedData::findIndexesInCpGList(cpg_list, bedmethyl.records()));
   }
   std::pair<RowIndexes, RowIndexes> overlapping_indexes{
       BedData::findOverLappingIndexes(reference_matrix.records(),
                                       bedmethyl.records())};

   if (overlapping_indexes.first.empty() ||
       overlapping_indexes.second.empty()) {
      throw std::runtime_error(
          "No overlapping indexes found between reference matrix and input "
          "bedmethyl file.");
   }
   reference_matrix.subsetRows(overlapping_indexes.first);
   bedmethyl.subsetRows(overlapping_indexes.second);

   reference_matrix.addMoreCellTypes(additional_cell_types);
}
}  // namespace Hylord::Processing
