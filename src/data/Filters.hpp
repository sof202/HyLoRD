#ifndef FILTERS_H_
#define FILTERS_H_

/**
 * @file    Filters.hpp
 * @brief   Declares filters that can be used when reading in files.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <stdexcept>
#include <vector>

#include "cli.hpp"
#include "types.hpp"

namespace Hylord::Filters {
using RowFilter = Hylord::IO::RowFilter;

class FilterCombiner {
  public:
   void addFilter(RowFilter filter) { m_filters.push_back(std::move(filter)); }
   RowFilter combinedFilter() const;

  private:
   std::vector<RowFilter> m_filters;
};

RowFilter generateFullRowFilter(const CMD::HylordConfig& config);

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
