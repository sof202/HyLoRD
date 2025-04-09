/**
 * @file    BedRecords.hpp
 * @brief   Describes record structures of UCSC BED files
 * @license MIT (See LICENSE file in the project root)
 */

#ifndef BEDRECORDS_H_
#define BEDRECORDS_H_

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

using Fields = std::vector<std::string>;

/**
 * @namespace BedRecords
 * @brief Namespace containing BED format record parsers and related utilities.
 *
 * All record types support conversion from TSV fields via static `fromFields`
 * methods, making them compatible with the TSVRecord concept and
 * TSVFileReader.
 */
namespace BedRecords {

/**
 * @brief Converts chromosome string to standardized numeric representation.
 *
 * @param chr A string view of the form "chrXXX" where XXX is the chromosome
 *            identifier (e.g., "chr1", "chrX", "chrM")
 * @return int Numeric chromosome representation:
 *             - 1-22 for autosomes
 *             - 23 for X
 *             - 24 for Y
 *             - 25 for M (mitochondrial)
 * @throw std::invalid_argument If chromosome format is invalid
 * @note Handles common chromosome naming conventions ("chr1", "Chr1", "CHR1")
 */
int parseChromosomeNumber(const std::string_view chr);

/**
 * @brief Validates that fields meet minimum requirements for BED records.
 *
 * @param fields Vector of string fields from TSV parsing
 * @param min_expected_fields Minimum number of required fields
 * @throw std::runtime_error If fields don't meet requirements
 */
void validateFields(const Fields& fields, int min_expected_fields);

struct Bed {
   int chromosome{1};
   int start{};
   int end{};
   char name{};  // expected m or h

   static void parseCoreFields(Bed& core, const Fields& fields) {
      validateFields(fields, 4);
      core.chromosome = parseChromosomeNumber(fields[0]);
      core.start = std::stoi(fields[1]);
      core.end = std::stoi(fields[2]);
      core.name = fields[3][0];
   }
};

struct Bed4 : public Bed {
   static Bed4 fromFields(const Fields& fields) {
      Bed4 parsed_row{};
      parseCoreFields(parsed_row, fields);
      return parsed_row;
   }
};

struct Bed4PlusX : public Bed {
   std::vector<double> methylation_percentages{};

   static Bed4PlusX fromFields(const Fields& fields) {
      validateFields(fields, 5);
      Bed4PlusX parsed_row{};
      parseCoreFields(parsed_row, fields);
      for (std::size_t i{4}; i < fields.size(); ++i) {
         parsed_row.methylation_percentages.emplace_back(std::stod(fields[i]));
      }
      return parsed_row;
   }
};

struct Bed9Plus9 : public Bed {
   // There are more fields in Bed9+9 files, but they aren't useful here
   double methylation_percentage{};

   static Bed9Plus9 fromFields(const Fields& fields) {
      validateFields(fields, 5);
      Bed9Plus9 parsed_row{};
      parseCoreFields(parsed_row, fields);
      parsed_row.methylation_percentage = std::stod(fields[4]);
      return parsed_row;
   }
};
}  // namespace BedRecords

#endif
