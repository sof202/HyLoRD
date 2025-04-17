#ifndef FILTERS_H_
#define FILTERS_H_

/**
 * @file    Filters.hpp
 * @brief   Declares filters that can be used when reading in files.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <vector>

#include "cli.hpp"
#include "types.hpp"

namespace Hylord::Filters {
using RowFilter = Hylord::IO::RowFilter;

class FilterCombiner {
  public:
   void addFilter(RowFilter filter) { m_filters.push_back(std::move(filter)); }
   RowFilter combinedFilter() const;
   bool empty() { return m_filters.empty(); }

  private:
   std::vector<RowFilter> m_filters;
};

RowFilter generateNameFilter(const CMD::HylordConfig& config);
RowFilter generateBedmethylRowFilter(const CMD::HylordConfig& config);

}  // namespace Hylord::Filters

#endif
