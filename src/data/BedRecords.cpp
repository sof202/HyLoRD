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
int parseChromosomeNumber(const std::string_view chr) {
   size_t start_pos = 0;
   if (chr.size() >= 3 && std::tolower(chr[0]) == 'c' &&
       std::tolower(chr[1]) == 'h' && std::tolower(chr[2]) == 'r') {
      start_pos = 3;
   }

   std::string_view number_part = chr.substr(start_pos);

   if (number_part.size() == 1) {
      const char c = std::tolower(number_part[0]);
      if (c == 'x') return 23;
      if (c == 'y') return 24;
      if (c == 'm') return 25;
   }
   if (std::all_of(number_part.begin(), number_part.end(), ::isdigit)) {
      return std::stoi(std::string(number_part));
   }
   throw std::runtime_error("Failed to glean chromosome number for: " +
                            std::string(chr));
}

void validateFields(const Fields& fields, int min_expected_fields) {
   if (static_cast<int>(std::ssize(fields)) < min_expected_fields) {
      throw std::out_of_range(
          "Could not parse field, too few fields (expected >=" +
          std::to_string(min_expected_fields) + ")");
   }
}
}  // namespace Hylord::BedRecords
