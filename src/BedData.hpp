#ifndef BEDDATA_H_
#define BEDDATA_H_

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "BedRecords.hpp"

using RowFilterFunction = std::function<bool(const std::vector<std::string>&)>;
using ColumnIndexes = std::vector<std::size_t>;
using RowIndexes = std::vector<std::size_t>;

namespace Hylord {
template <typename RecordType>
void subsetRows(std::vector<RecordType>& records, const RowIndexes& rows) {
   std::vector<RecordType> subset_records;
   subset_records.reserve(rows.size());

   for (auto i : rows) {
      if (i >= records.size()) throw std::out_of_range("Invalid row index.");
      subset_records.push_back(std::move(records[i]));
   }
   records = std::move(subset_records);
}
}  // namespace Hylord

class CpGData {
  public:
   CpGData(std::vector<BedRecords::Bed4> records) :
       m_records{std::move(records)} {}

   const std::vector<BedRecords::Bed4>& records() { return m_records; }
   void subsetRows(RowIndexes rows) { Hylord::subsetRows(m_records, rows); };

  private:
   std::vector<BedRecords::Bed4> m_records{};
};

class ReferenceMatrixData {
  public:
   ReferenceMatrixData(std::vector<BedRecords::Bed4PlusX> records) :
       m_records{std::move(records)} {}

   const std::vector<BedRecords::Bed4PlusX>& records() { return m_records; }
   void subsetRows(RowIndexes rows) { Hylord::subsetRows(m_records, rows); };

  private:
   std::vector<BedRecords::Bed4PlusX> m_records{};
};

class BedMethylData {
  public:
   BedMethylData(std::vector<BedRecords::Bed9Plus9> records) :
       m_records{std::move(records)} {}

   const std::vector<BedRecords::Bed9Plus9>& records() { return m_records; }
   void subsetRows(RowIndexes rows) { Hylord::subsetRows(m_records, rows); };

  private:
   std::vector<BedRecords::Bed9Plus9> m_records{};
};

#endif
