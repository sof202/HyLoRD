#ifndef BEDRECORDS_H_
#define BEDRECORDS_H_

/**
 * @file    BedRecords.hpp
 * @brief  Describes record structures of UCSC BED files
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */
#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "maths/percentage.hpp"
#include "types.hpp"

/**
 * Parsers for BED genomic data formats (BED4, BED4+, BED9+9)
 *
 * All record types implement fromFields() for TSVRecord compatibility.
 */
namespace Hylord::BedRecords {
/// Parses a chromosome string into its numeric representation.
auto parseChromosomeNumber(std::string_view chr) -> int;

/// Validates that a Fields container meets minimum field count
/// requirements.
void validateFields(const Fields& fields, int min_expected_fields);

/// Core BED fields shared by all variants
struct Bed {
   int chromosome{1};
   int start{};
   char name{};  // expected m or h

   /**
    * Parses core BED fields (chromosome, start position, and name) from
    * a given fields container.
    *
    * Requires at least 4 fields in the input container. Validates fields
    * before parsing. Uses parseChromosomeNumber() for chromosome string
    * conversion.
    * @throws std::out_of_range if fields container has fewer than 4 elements.
    * @throws std::runtime_error if chromosome parsing fails.
    */
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

   /**
    * Constructs a Bed4PlusX record from TSV fields containing methylation
    * data.
    *
    * Parses and validates input fields to create a Bed4PlusX record with
    * multiple methylation proportions. Processes all fields beyond the core 4
    * as methylation proportions, converting each to a proportional value.
    * @throws std::invalid_argument if field validation fails
    * @throws std::out_of_range if string conversion fails for any methylation
    * value
    */
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

   /**
    * Constructs a Bed9Plus9 record from TSV fields.
    *
    * Parses and validates the input fields to create a Bed9Plus9 record.
    * Converts the methylation proportion field from string to a proportion
    * value.
    * @throws std::invalid_argument if field validation fails
    * @throws std::out_of_range if string conversion fails
    */
   static auto fromFields(const Fields& fields) -> Bed9Plus9 {
      validateFields(fields, 6);
      Bed9Plus9 parsed_row{};
      parseCoreFields(parsed_row, fields);
      parsed_row.methylation_proportion =
          Maths::convertToProportion(std::stod(fields[5]));
      return parsed_row;
   }
};

/// Newline separated list of cell types
struct CellType {
   std::string cell_type;
   static auto fromFields(const Fields& fields) -> CellType {
      if (fields[0].empty())
         throw std::runtime_error("Failed to parse fields (empty).");

      return CellType{fields[0]};
   }
};
}  // namespace Hylord::BedRecords

#endif
