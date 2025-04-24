#ifndef BEDRECORDS_H_
#define BEDRECORDS_H_

/**
 * @file    BedRecords.hpp
 * @brief   Describes record structures of UCSC BED files
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include "maths/percentage.hpp"
#include "types.hpp"

/**
 * @namespace BedRecords
 * @brief Parsers for BED genomic data formats (BED4, BED4+, BED9+9)
 *
 * All record types implement fromFields() for TSVRecord compatibility.
 */
namespace Hylord::BedRecords {
/** Converts "chrX" format strings to numeric values (1-22=autosomes, 23=X,
 * 24=Y, 25=M)
 */
auto parseChromosomeNumber(std::string_view chr) -> int;

/// Validates minimum field count and basic format requirements
void validateFields(const Fields& fields, int min_expected_fields);

/**
 * @brief Core BED fields shared by all variants
 * @details Chromosome, start position, and feature name.
 * parseCoreFields() handles common parsing logic for derived types.
 * End position is not accounted for because it is never used
 */
struct Bed {
   int chromosome{1};
   int start{};
   char name{};  // expected m or h

   static void parseCoreFields(Bed& core, const Fields& fields) {
      validateFields(fields, 4);
      core.chromosome = parseChromosomeNumber(fields[0]);
      core.start = std::stoi(fields[1]);
      core.name = fields[3][0];
   }
};

/// Standard BED4 format (chrom, start, end, name)
struct Bed4 : public Bed {
   static auto fromFields(const Fields& fields) -> Bed4 {
      Bed4 parsed_row{};
      parseCoreFields(parsed_row, fields);
      return parsed_row;
   }
};

/// BED4+ with variable-length methylation percentages (reference matrix)
struct Bed4PlusX : public Bed {
   std::vector<double> methylation_proportions;

   static auto fromFields(const Fields& fields) -> Bed4PlusX {
      validateFields(fields, 5);
      Bed4PlusX parsed_row{};
      parseCoreFields(parsed_row, fields);
      for (std::size_t i{4}; i < fields.size(); ++i) {
         parsed_row.methylation_proportions.emplace_back(
             Maths::convertToProportion(std::stod(fields[i])));
      }
      return parsed_row;
   }
};

/// BED9+9 format (uses first methylation value only)
struct Bed9Plus9 : public Bed {
   double methylation_proportion{};

   static auto fromFields(const Fields& fields) -> Bed9Plus9 {
      validateFields(fields, 6);
      Bed9Plus9 parsed_row{};
      parseCoreFields(parsed_row, fields);
      parsed_row.methylation_proportion =
          Maths::convertToProportion(std::stod(fields[5]));
      return parsed_row;
   }
};
}  // namespace Hylord::BedRecords

#endif
