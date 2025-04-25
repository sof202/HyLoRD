#ifndef BEDDATA_H_
#define BEDDATA_H_

/**
 * @file    BedData.hpp
 * @brief   Defines data storage containers for certain bed files.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <numeric>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "Eigen/Dense"
#include "concepts.hpp"
#include "data/BedRecords.hpp"
#include "types.hpp"

/// Defines containers for holding data from bed files
namespace Hylord::BedData {
/**
 * Splits a TSV (Tab-Separated Values) or space-delimited line into individual
 * fields.
 *
 * Parses a string containing tab or space delimited values, extracting each
 * field into a vector. The function handles consecutive delimiters and
 * includes the final field in the result. Both tabs and spaces are treated as
 * valid delimiters.
 */
template <Records::TSVRecord RecordType>
void subset(Records::Collection<RecordType>& records, const RowIndexes& rows) {
   using Records = Records::Collection<RecordType>;
   Records subset_records;
   subset_records.reserve(rows.size());

   for (auto row : rows) {
      if (row >= records.size()) throw std::out_of_range("Invalid row index.");
      subset_records.push_back(std::move(records[row]));
   }
   records = std::move(subset_records);
}

/// Container for CpG list data
class CpGData {
  public:
   CpGData() = default;
   CpGData(std::vector<BedRecords::Bed4> records) :
       m_records{std::move(records)} {}

   [[nodiscard]] auto records() const -> const std::vector<BedRecords::Bed4>& {
      return m_records;
   }
   [[nodiscard]] auto empty() const -> bool { return m_records.empty(); }
   void subsetRows(const RowIndexes& rows) { subset(m_records, rows); };

  private:
   std::vector<BedRecords::Bed4> m_records;
};

/// Container for bedmethyl data
class BedMethylData {
  public:
   BedMethylData() = default;
   BedMethylData(std::vector<BedRecords::Bed9Plus9> records) :
       m_records{std::move(records)} {}

   [[nodiscard]] auto records() const
       -> const std::vector<BedRecords::Bed9Plus9>& {
      return m_records;
   }
   [[nodiscard]] auto empty() const -> bool { return m_records.empty(); }
   void subsetRows(const RowIndexes& rows) { subset(m_records, rows); };
   /// Converts methylation proportions from BED records into an Eigen vector.
   [[nodiscard]] auto getAsEigenVector() const -> Vector;

  private:
   std::vector<BedRecords::Bed9Plus9> m_records;
};

/// Container for reference matrix data
class ReferenceMatrixData {
  public:
   ReferenceMatrixData() = default;
   ReferenceMatrixData(std::vector<BedRecords::Bed4PlusX> records) :
       m_records{std::move(records)} {}
   ReferenceMatrixData(const BedMethylData& bedmethyl) {
      for (const auto& row : bedmethyl.records()) {
         m_records.push_back(
             BedRecords::Bed4PlusX{row.chromosome, row.start, row.name, {}});
      }
   }

   [[nodiscard]] auto records() const
       -> const std::vector<BedRecords::Bed4PlusX>& {
      return m_records;
   }
   [[nodiscard]] auto empty() const -> bool { return m_records.empty(); }
   void subsetRows(const RowIndexes& rows) { subset(m_records, rows); };
   /// Adds additional cell types to the reference matrix with randomized
   /// methylation/hydroxymethylation values.
   void addMoreCellTypes(int num_cell_types);
   [[nodiscard]] auto numberOfCellTypes() const -> int {
      return static_cast<int>(
          std::ssize(m_records[0].methylation_proportions));
   }
   /// Converts the reference matrix data into an Eigen matrix format.
   [[nodiscard]] auto getAsEigenMatrix() const -> Matrix;

  private:
   std::vector<BedRecords::Bed4PlusX> m_records;
};

/**
 * Finds overlapping indexes between two BED files using a two-pointer
 * approach.
 *
 * Compares two BED files (assumed to be sorted) and returns pairs of indexes
 * where records match. Records are considered matching if their chromosome,
 * start position, and name fields are equal.
 */
template <typename BedTypeOne, typename BedTypeTwo>
auto findOverLappingIndexes(const BedTypeOne& bed_one,
                            const BedTypeTwo& bed_two)
    -> std::pair<RowIndexes, RowIndexes> {
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

/**
 * Finds indexes of BED entries that match records in a CpG list using binary
 * search.
 *
 * Searches for BED entries that match CpG records by chromosome, start
 * position, and name.
 * @throws std::runtime_error if no overlapping records are found between the
 * CpG list and BED entries.
 */
template <typename Records>
auto findIndexesInCpGList(const BedData::CpGData& cpg_list,
                          const Records& bed_entries) -> RowIndexes {
   const std::vector<BedRecords::Bed4>& cpgs{cpg_list.records()};
   RowIndexes bed_indexes_in_cpg_list{};
   bed_indexes_in_cpg_list.reserve(cpgs.size());

   for (RowIndex cpg{}; cpg < cpgs.size(); ++cpg) {
      auto cpg_key{
          std::tie(cpgs[cpg].chromosome, cpgs[cpg].start, cpgs[cpg].name)};
      RowIndex low{};
      RowIndex high{static_cast<RowIndex>(bed_entries.size() - 1)};
      while (low <= high) {
         RowIndex mid{std::midpoint(low, high)};
         auto row_key{std::tie(bed_entries[mid].chromosome,
                               bed_entries[mid].start,
                               bed_entries[mid].name)};
         if (row_key == cpg_key) {
            bed_indexes_in_cpg_list.push_back(mid);
            break;
         }
         if (row_key < cpg_key) {
            low = mid + 1;
         } else {
            high = mid - 1;
         }
      }
   }
   if (bed_indexes_in_cpg_list.empty())
      throw std::runtime_error("No row overlap with cpg_list.");
   return bed_indexes_in_cpg_list;
}
}  // namespace Hylord::BedData

#endif
