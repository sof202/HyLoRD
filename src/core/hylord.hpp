#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include <cstddef>
#include <iostream>
#include <numeric>
#include <string_view>
#include <tuple>

#include "data/BedData.hpp"
#include "data/BedRecords.hpp"
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

template <typename Records>
RowIndexes findIndexesInCpGList(const BedData::CpGData& cpg_list,
                                const Records& bed_entries) {
   const std::vector<BedRecords::Bed4>& cpgs{cpg_list.records()};
   RowIndexes bed_indexes_in_cpg_list{};
   bed_indexes_in_cpg_list.reserve(cpgs.size());

   for (RowIndex cpg{}; cpg < cpgs.size(); ++cpg) {
      auto cpg_key{
          std::tie(cpgs[cpg].chromosome, cpgs[cpg].start, cpgs[cpg].name)};
      RowIndex low{};
      RowIndex high{bed_entries.size() - 1};
      while (low <= high) {
         RowIndex mid{std::midpoint(low, high)};
         auto row_key{std::tie(bed_entries[mid].chromosome,
                               bed_entries[mid].start,
                               bed_entries[mid].name)};
         if (row_key == cpg_key) {
            bed_indexes_in_cpg_list.push_back(mid);
            break;
         } else if (row_key < cpg_key) {
            low = mid + 1;
         } else {
            high = mid - 1;
         }
      }
   }
   return bed_indexes_in_cpg_list;
}

void run(const std::string_view bedmethyl_file,
         const std::string_view reference_matrix_file,
         const std::string_view cpg_list_file,
         const std::string_view cell_type_list_file,
         const int additional_cell_types,
         const int threads);

}  // namespace Hylord

#endif
