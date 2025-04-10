#ifndef FILTERS_H_
#define FILTERS_H_

#include <stdexcept>
#include <vector>

#include "types.hpp"

namespace Hylord::Filters {
using RowFilter = Hylord::IO::RowFilter;

class FilterCombiner {
  public:
   void addFilter(RowFilter filter) { m_filters.push_back(filter); }
   bool operator()(const Fields& row) const {
      for (const auto& filter : m_filters) {
         if (!filter(row)) return false;
      }
      return true;
   }

  private:
   std::vector<RowFilter> m_filters;
};

template <int min_reads>
bool low_read_filter(const Fields& fields) {
   if (fields.size() < 5) {
      throw std::out_of_range(
          "Could not apply row filter, not enough fields.");
   }
   return std::stoi(fields[4]) > min_reads;
}

template <int max_reads>
bool high_read_filter(const Fields& fields) {
   if (fields.size() < 5) {
      throw std::out_of_range(
          "Could not apply row filter, not enough fields.");
   }
   return std::stoi(fields[4]) < max_reads;
}

inline bool is_hydroxy_read(const Fields& fields) {
   if (fields.size() < 4) {
      throw std::out_of_range(
          "Could not apply row filter, not enough fields.");
   }
   return fields[3][0] == 'h';
}

inline bool is_methyl_read(const Fields& fields) {
   if (fields.size() < 4) {
      throw std::out_of_range(
          "Could not apply row filter, not enough fields.");
   }
   return fields[3][0] == 'm';
}

}  // namespace Hylord::Filters

#endif
