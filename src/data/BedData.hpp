#ifndef BEDDATA_H_
#define BEDDATA_H_

#include <stdexcept>
#include <vector>

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

  private:
   std::vector<BedRecords::Bed9Plus9> m_records{};
};
}  // namespace Hylord::BedData

#endif
