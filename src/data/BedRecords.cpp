/**
 * @file    BedRecords.cpp
 * @brief   Defines functions for parsing/validating fields of BED files
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/BedRecords.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace Hylord::BedRecords {
/**
 * Handles both numeric chromosome formats (e.g., "1", "22") and prefixed
 * formats (e.g., "chr1"). Also supports special chromosomes (X, Y, M) by
 * converting them to numeric values (23, 24, 25).
 *
 * @throws std::runtime_error if the chromosome string cannot be parsed.
 */
auto parseChromosomeNumber(const std::string_view chr) -> int {
   size_t start_pos = 0;

   // Handles cases where bed file has values in the chromosome column (column
   // one) with 'chrXXX' or just 'XXX'.
   if (chr.size() >= 3 && std::tolower(chr[0]) == 'c' &&
       std::tolower(chr[1]) == 'h' && std::tolower(chr[2]) == 'r') {
      start_pos = 3;
   }

   std::string_view number_part = chr.substr(start_pos);

   if (std::ranges::all_of(number_part, ::isdigit)) {
      return std::stoi(std::string(number_part));
   }
   if (number_part.size() == 1) {
      const char chromosome_letter{
          static_cast<char>(std::tolower(number_part[0]))};
      if (chromosome_letter == 'x') return 23;
      if (chromosome_letter == 'y') return 24;
      if (chromosome_letter == 'm') return 25;
   }
   throw std::runtime_error("Failed to glean chromosome number for: " +
                            std::string(chr));
}

/**
 * Checks if the number of fields is at least the specified minimum expected.
 *
 * @throws std::out_of_range if the field count is less than the required
 * minimum.
 */
void validateFields(const Fields& fields, int min_expected_fields) {
   if (static_cast<int>(std::ssize(fields)) < min_expected_fields) {
      throw std::out_of_range(
          "Could not parse field, too few fields (expected >=" +
          std::to_string(min_expected_fields) + ")");
   }
}
}  // namespace Hylord::BedRecords
