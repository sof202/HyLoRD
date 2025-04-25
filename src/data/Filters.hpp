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

/// Defines filtering utilities used when reading input files
namespace Hylord::Filters {
using RowFilter = Hylord::IO::RowFilter;

/**
 * Combines multiple row filters into single composite filter
 *
 * Provides functionality to combine multiple row filtering conditions.
 * The class maintains a collection of filters that can be applied
 * sequentially.
 * - Filters are added via addFilter() and stored in execution order
 * - combinedFilter() generates a single filter that applies all conditions
 * - empty() checks if any filters have been added
 * The combined filter will only pass rows that satisfy all constituent
 * filters.
 *
 * ### Example usage
 * @code
 * FilterCombiner filter{};
 * filter.addFilter([](const Fields& fields){return fields[1] > 0;});
 * filter.addFilter([](const Fields& fields){return fields[2] < 0;});
 * RowFilter combined_filter{filter.combinedFilter()};
 * @endcode
 */
class FilterCombiner {
  public:
   void addFilter(RowFilter filter) { m_filters.push_back(std::move(filter)); }
   /// Combines all stored filters into a single composite filter function.
   [[nodiscard]] auto combinedFilter() const -> RowFilter;
   auto empty() -> bool { return m_filters.empty(); }

  private:
   std::vector<RowFilter> m_filters;
};

/// Generates a composite row filter if only methylation or hydroxymethylation
/// is desired
auto generateNameFilter(const CMD::HylordConfig& config) -> RowFilter;

/// Generates a composite filter for bedmethyl rows based on configuration
/// given on command line
auto generateBedmethylRowFilter(const CMD::HylordConfig& config) -> RowFilter;

}  // namespace Hylord::Filters

#endif
