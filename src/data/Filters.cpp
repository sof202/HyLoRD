/**
 * @file    Filters.cpp
 * @brief   Defines filters that can be used when reading in files.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/Filters.hpp"

#include <limits>

#include "cli.hpp"
#include "types.hpp"

namespace Hylord::Filters {
RowFilter FilterCombiner::combinedFilter() const {
   return {[filters = m_filters](const Fields& row) {
      return std::ranges::all_of(
          filters, [&](const auto& filter) { return filter(row); });
   }};
}

RowFilter makeLowReadFilter(int min_reads) {
   return [min_reads](const Fields& fields) -> bool {
      if (fields.size() < 5) {
         throw std::out_of_range(
             "Could not apply row filter, not enough fields.");
      }
      return std::stoi(fields[4]) > min_reads;
   };
}

RowFilter makeHighReadFilter(int max_reads) {
   return [max_reads](const Fields& fields) -> bool {
      if (fields.size() < 5) {
         throw std::out_of_range(
             "Could not apply row filter, not enough fields.");
      }
      return std::stoi(fields[4]) < max_reads;
   };
}

RowFilter generateFullRowFilter(const CMD::HylordConfig& config) {
   FilterCombiner combined_filters{};
   if (config.min_read_depth != 0)
      combined_filters.addFilter(makeLowReadFilter(config.min_read_depth));
   if (config.max_read_depth != std::numeric_limits<int>::max())
      combined_filters.addFilter(makeHighReadFilter(config.max_read_depth));

   return combined_filters.empty() ? nullptr
                                   : combined_filters.combinedFilter();
}

}  // namespace Hylord::Filters
