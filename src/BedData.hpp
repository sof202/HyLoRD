#ifndef BEDDATA_H_
#define BEDDATA_H_

#include <functional>
#include <string>
#include <vector>

using RowFilterFunction = std::function<bool(const std::vector<std::string>&)>;
using ColumnIndexes = std::vector<std::size_t>;

template <typename RecordType>
class BedData {
  public:
   BedData(std::vector<RecordType> records) : m_records{std::move(records)} {}
   const std::vector<RecordType>& records() const { return m_records; }

  private:
   std::vector<RecordType> m_records{};
};

#endif
