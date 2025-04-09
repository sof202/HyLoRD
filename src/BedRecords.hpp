#ifndef BEDRECORDS_H_
#define BEDRECORDS_H_

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

using Fields = std::vector<std::string>;

namespace BedRecords {
int parseChromosomeNumber(const std::string_view chr);
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
