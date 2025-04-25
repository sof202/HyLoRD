/**
 * @file    Filters.cpp
 * @brief   Defines filters that can be used when reading in files.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/Filters.hpp"

#include <limits>
#include <stdexcept>

#include "cli.hpp"
#include "types.hpp"

namespace Hylord::Filters {
/**
 * Creates a new RowFilter that applies all stored filters in sequence.
 * The combined filter will:
 * 1. Return true only if ALL constituent filters pass the input row
 * 2. Short-circuit evaluation (stops after first failing filter)
 * 3. Maintain the original filter application order
 * @return A new RowFilter that performs logical AND of all component filters
 */
[[nodiscard]] auto FilterCombiner::combinedFilter() const -> RowFilter {
   return {[filters = m_filters](const Fields& row) {
      return std::ranges::all_of(
          filters, [&](const auto& filter) { return filter(row); });
   }};
}

auto makeLowReadFilter(int min_reads) -> RowFilter {
   return [min_reads](const Fields& fields) -> bool {
      if (fields.size() < 5) {
         throw std::out_of_range(
             "Could not apply row filter, not enough fields.");
      }
      return std::stoi(fields[4]) > min_reads;
   };
}

auto makeHighReadFilter(int max_reads) -> RowFilter {
   return [max_reads](const Fields& fields) -> bool {
      if (fields.size() < 5) {
         throw std::out_of_range(
             "Could not apply row filter, not enough fields.");
      }
      return std::stoi(fields[4]) < max_reads;
   };
}

const RowFilter is_hydroxy_read{[](const Fields& fields) {
   if (fields.size() < 4) {
      throw std::out_of_range(
          "Could not apply row filter, not enough fields.");
   }
   return fields[3][0] == 'h';
}};

const RowFilter is_methyl_read{[](const Fields& fields) {
   if (fields.size() < 4) {
      throw std::out_of_range(
          "Could not apply row filter, not enough fields.");
   }
   return fields[3][0] == 'm';
}};

auto generateNameFilter(const CMD::HylordConfig& config) -> RowFilter {
   FilterCombiner combined_filters{};
   if (config.use_only_methylation_signal)
      combined_filters.addFilter(is_methyl_read);
   if (config.use_only_hydroxy_signal)
      combined_filters.addFilter(is_hydroxy_read);

   return combined_filters.empty() ? nullptr
                                   : combined_filters.combinedFilter();
}

auto generateBedmethylRowFilter(const CMD::HylordConfig& config) -> RowFilter {
   FilterCombiner combined_filters{};
   if (config.min_read_depth != 0)
      combined_filters.addFilter(makeLowReadFilter(config.min_read_depth));
   if (config.max_read_depth != std::numeric_limits<int>::max())
      combined_filters.addFilter(makeHighReadFilter(config.max_read_depth));
   if (config.use_only_methylation_signal)
      combined_filters.addFilter(is_methyl_read);
   if (config.use_only_hydroxy_signal)
      combined_filters.addFilter(is_hydroxy_read);

   return combined_filters.empty() ? nullptr
                                   : combined_filters.combinedFilter();
}
}  // namespace Hylord::Filters
