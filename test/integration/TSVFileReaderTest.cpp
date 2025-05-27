#include <gtest/gtest.h>
#include <sys/stat.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <ratio>
#include <stdexcept>
#include <string>
#include <vector>

#include "HylordException.hpp"
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
   class Timer {
     private:
      using Clock = std::chrono::steady_clock;
      using Second = std::chrono::duration<double, std::ratio<1>>;

     public:
      void reset() { m_begin = Clock::now(); }
      [[nodiscard]] auto elpased() const -> double {
         return std::chrono::duration_cast<Second>(Clock::now() - m_begin)
             .count();
      }

     private:
      std::chrono::time_point<Clock> m_begin{Clock::now()};
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

TEST_F(TSVReaderIntegrationTest, CorrectlyAppliesRowFilter) {
   // File contains many rows, only 2 of which don't contain a '2' in first
   // field
   IO::TSVFileReader<TwoNumbers> reader{
       getTestPath("valid/row_filter.tsv"),
       {0, 1},
       [](const Fields& fields) -> bool { return std::stoi(fields[0]) != 2; }};
   reader.load();
   std::vector<TwoNumbers> rows{reader.extractRecords()};
   EXPECT_EQ(rows.size(), 2);
}

TEST_F(TSVReaderIntegrationTest, RecordAccessThrowsIfNotLoaded) {
   IO::TSVFileReader<TwoNumbers> reader{getTestPath("valid/two_numbers.tsv")};
   EXPECT_THROW(reader.extractRecords(), std::runtime_error);
}

TEST_F(TSVReaderIntegrationTest, ThrowsIfFileLoadedTwice) {
   IO::TSVFileReader<TwoNumbers> reader{getTestPath("valid/two_numbers.tsv")};
   reader.load();
   EXPECT_THROW(reader.load(), HylordException);
}

TEST_F(TSVReaderIntegrationTest, ThrowsOnInvalidPermissions) {
   std::string data_path{getTestPath("invalid/invalid_permissions.tsv")};
   std::ofstream(data_path) << "test\n";
   chmod(data_path.c_str(), 0000);

   EXPECT_THROW(IO::TSVFileReader<TwoNumbers> reader{data_path};
                , FileReadException);
}

TEST_F(TSVReaderIntegrationTest, ThowsOnNonExistantFile) {
   EXPECT_THROW(
       IO::TSVFileReader<TwoNumbers> reader{"this_file_does_not_exist.tsv"};
       , FileReadException);
}

TEST_F(TSVReaderIntegrationTest, ThowsOnEmptyFile) {
   EXPECT_THROW(IO::TSVFileReader<TwoNumbers> reader{"invalid/empty.tsv"};
                , FileReadException);
}

TEST_F(TSVReaderIntegrationTest, SkipsEmptyLines) {
   IO::TSVFileReader<TwoNumbers> reader{getTestPath("valid/empty_lines.tsv")};
   reader.load();
   std::vector<TwoNumbers> rows{reader.extractRecords()};
   EXPECT_EQ(rows[0].num1, 1);
   EXPECT_EQ(rows[1].num2, 4);
}

TEST_F(TSVReaderIntegrationTest, SkipsMalformedLines) {
   IO::TSVFileReader<TwoNumbers> reader{
       getTestPath("valid/malformed_lines.tsv")};
   reader.load();
   std::vector<TwoNumbers> rows{reader.extractRecords()};
   EXPECT_EQ(rows[0].num1, 1);
   EXPECT_EQ(rows[1].num2, 4);
}

TEST_F(TSVReaderIntegrationTest, PerformanceCheck) {
   std::string data_path{getTestPath("valid/long_file.tsv")};
   constexpr int n_rows{250000};
   {
      std::ofstream long_file(data_path);
      for (int i{}; i < n_rows; ++i) {
         long_file << "1\t2\n";
      }
   }  // About 1MB

   IO::TSVFileReader<TwoNumbers> reader{data_path};
   Timer timer;
   reader.load();
   // With extrapolation, 1GB should take less than 100s to parse
   const auto time_taken_seconds{timer.elpased()};
   constexpr double max_acceptable_time_seconds{0.1};
   EXPECT_LE(time_taken_seconds, max_acceptable_time_seconds);
   std::cout << "Reading in 1MB file took: " << time_taken_seconds
             << "seconds\n";
}
}  // namespace Hylord
