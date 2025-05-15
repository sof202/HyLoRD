#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "io/TSVFileReader.hpp"
#include "types.hpp"

namespace Hylord {
class TSVReaderIntegrationTest : public ::testing::Test {
  protected:
   static auto getTestPath(const std::string& file_name) -> std::string {
      static std::string test_dir{TEST_DATA_DIR};
      return test_dir + '/' + file_name;
   }
   struct TwoNumbers {
      int num1{};
      int num2{};

      static auto fromFields(const Fields& fields) {
         TwoNumbers parsed_row{};
         parsed_row.num1 = std::stoi(fields[0]);
         parsed_row.num2 = std::stoi(fields[1]);
         return parsed_row;
      }
   };
};

TEST_F(TSVReaderIntegrationTest, ReadsSimpleFiles) {
   IO::TSVFileReader<TwoNumbers> reader{getTestPath("valid/two_numbers.tsv")};
   reader.load();
   std::vector<TwoNumbers> rows{reader.extractRecords()};
   EXPECT_EQ(rows[0].num1, 1);
   EXPECT_EQ(rows[1].num2, 4);
}

TEST_F(TSVReaderIntegrationTest, ExtractsDesiredFields) {
   IO::TSVFileReader<TwoNumbers> reader{getTestPath("valid/three_numbers.tsv"),
                                        {0, 2}};
   reader.load();
   std::vector<TwoNumbers> rows{reader.extractRecords()};
   EXPECT_EQ(rows[0].num1, 1);
   EXPECT_EQ(rows[1].num2, 6);
}

}  // namespace Hylord
