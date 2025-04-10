/**
 * @file    BedRecords.cpp
 * @brief   Defines functions for parsing/validating fields of BED files
 * @license MIT (See LICENSE file in the repository root)
 */

#include "data/BedRecords.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Hylord::BedRecords {
int parseChromosomeNumber(const std::string_view chr) {
   std::string chromosomeNumber{chr.substr(3, chr.size())};
   std::transform(chromosomeNumber.begin(),
                  chromosomeNumber.end(),
                  chromosomeNumber.begin(),
                  [](unsigned char c) { return std::tolower(c); });
   if (chromosomeNumber == "x") return 23;
   if (chromosomeNumber == "y") return 24;
   if (chromosomeNumber == "m") return 25;
   try {
      return std::stoi(chromosomeNumber);
   } catch (const std::invalid_argument& e) {
      std::cerr << "Could not obtain chromosome number for: " << chr << '\n';
      throw;
   }
}

void validateFields(const Fields& fields, int min_expected_fields) {
   if (static_cast<int>(std::ssize(fields)) < min_expected_fields) {
      throw std::runtime_error(
          "Could not parse field, too few fields (expected >=" +
          std::to_string(min_expected_fields) + ")");
   }
}
}  // namespace Hylord::BedRecords
