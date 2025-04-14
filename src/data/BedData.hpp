#ifndef BEDDATA_H_
#define BEDDATA_H_

#include <numeric>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "Eigen/Dense"
#include "concepts.hpp"
#include "data/BedRecords.hpp"
#include "types.hpp"

namespace Hylord::BedData {
template <Records::TSVRecord RecordType>
void subset(Records::Collection<RecordType>& records, const RowIndexes& rows) {
   using Records = Records::Collection<RecordType>;
   Records subset_records;
   subset_records.reserve(rows.size());

   for (auto i : rows) {
      if (i >= records.size()) throw std::out_of_range("Invalid row index.");
      subset_records.push_back(std::move(records[i]));
   }
   records = std::move(subset_records);
}

class CpGData {
  public:
   CpGData(std::vector<BedRecords::Bed4> records) :
       m_records{std::move(records)} {}

   const std::vector<BedRecords::Bed4>& records() const { return m_records; }
   void subsetRows(RowIndexes rows) { subset(m_records, rows); };

  private:
   std::vector<BedRecords::Bed4> m_records{};
};

class ReferenceMatrixData {
  public:
   ReferenceMatrixData(std::vector<BedRecords::Bed4PlusX> records) :
       m_records{std::move(records)} {}

   const std::vector<BedRecords::Bed4PlusX>& records() const {
      return m_records;
   }
   void subsetRows(RowIndexes rows) { subset(m_records, rows); };
   void addMoreCellTypes(int num_cell_types);
   Matrix getAsEigenMatrix() const;

  private:
   std::vector<BedRecords::Bed4PlusX> m_records{};
};

class BedMethylData {
  public:
   BedMethylData(std::vector<BedRecords::Bed9Plus9> records) :
       m_records{std::move(records)} {}

   const std::vector<BedRecords::Bed9Plus9>& records() const {
      return m_records;
   }
   void subsetRows(RowIndexes rows) { subset(m_records, rows); };
   Vector getAsEigenVector() const;

  private:
   std::vector<BedRecords::Bed9Plus9> m_records{};
};

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
}  // namespace Hylord::BedData

#endif
