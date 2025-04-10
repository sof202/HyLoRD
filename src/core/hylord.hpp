#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include <string_view>

#include "types.hpp"

namespace Hylord {
template <typename BedTypeOne, typename BedTypeTwo>
std::pair<RowIndexes, RowIndexes> findOverLappingIndexes(
    const BedTypeOne& bed_one, const BedTypeTwo& bed_two) {
   RowIndexes bed_one_overlapping_indexes{};
   RowIndexes bed_two_overlapping_indexes{};

   std::size_t min_indexes_to_keep{std::min(bed_one.size(), bed_two.size())};
   bed_one_overlapping_indexes.reserve(min_indexes_to_keep);
   bed_two_overlapping_indexes.reserve(min_indexes_to_keep);

   RowIndex bed_one_row{};
   RowIndex bed_two_row{};

   // We expect the majority of rows to match up already and bed files are
   // expected to be sorted (modkit will do this). So to find the indexes where
   // the two bed files overlap, we take a two pointer approach.
   while (bed_one_row < bed_one.size() && bed_two_row < bed_two.size()) {
      auto bed_one_key{std::tie(bed_one[bed_one_row].chromosome,
                                bed_one[bed_one_row].start,
                                bed_one[bed_one_row].name)};
      auto bed_two_key{std::tie(bed_two[bed_two_row].chromosome,
                                bed_two[bed_two_row].start,
                                bed_two[bed_two_row].name)};

      if (bed_one_key == bed_two_key) {
         bed_one_overlapping_indexes.push_back(bed_one_row);
         bed_two_overlapping_indexes.push_back(bed_two_row);
         bed_one_row++;
         bed_two_row++;
      } else if (bed_one_key < bed_two_key) {
         bed_one_row++;
      } else {
         bed_two_row++;
      }
   }
   return {bed_one_overlapping_indexes, bed_two_overlapping_indexes};
}

void run(const std::string_view bedmethyl_file,
         const std::string_view reference_matrix_file,
         const std::string_view cpg_list_file,
         const std::string_view cell_type_list_file,
         const int additional_cell_types,
         const int threads);

}  // namespace Hylord

#endif
