#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>

#include "data/BedRecords.hpp"
#include "types.hpp"

namespace Hylord {
using namespace std::string_view_literals;
TEST(ChromosomeParsingTest, BasicFunctionality) {
   EXPECT_EQ(BedRecords::parseChromosomeNumber("1"sv), 1);
   EXPECT_EQ(BedRecords::parseChromosomeNumber("chr2"sv), 2);
   EXPECT_EQ(BedRecords::parseChromosomeNumber("CHR10"sv), 10);
   EXPECT_EQ(BedRecords::parseChromosomeNumber("chrx"sv), 23);
   EXPECT_EQ(BedRecords::parseChromosomeNumber("chrY"sv), 24);
}

TEST(ChromosomeParsingTest, ThrowsOnInvalidChromosomeName) {
   EXPECT_THROW(BedRecords::parseChromosomeNumber("NC100012.2"),
                std::runtime_error);
   EXPECT_THROW(BedRecords::parseChromosomeNumber("chrt"), std::runtime_error);
}

TEST(Bed4ParsingTest, BasicFunctionality) {
   constexpr BedRecords::Bed4 expected_parsed_fields{1, 1000, 'h'};
   const Fields input_fields{"chr1", "1000", "1001", "h"};
   BedRecords::Bed4 actual_parsed_fields{
       BedRecords::Bed4::fromFields(input_fields)};
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
   EXPECT_EQ(actual_parsed_fields.start, expected_parsed_fields.start);
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
}

TEST(Bed4ParsingTest, HandlesFullSignalNames) {
   constexpr BedRecords::Bed4 expected_parsed_fields{1, 1000, 'h'};
   const Fields input_fields{"chr1", "1000", "1001", "hydroxymethylation"};
   BedRecords::Bed4 actual_parsed_fields{
       BedRecords::Bed4::fromFields(input_fields)};
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
}

TEST(Bed4ParsingTest, ThrowsOnTooFewFields) {
   const Fields input_fields{"chr1", "1000"};
   EXPECT_THROW(BedRecords::Bed4::fromFields(input_fields), std::out_of_range);
}

TEST(Bed4ParsingTest, ThrowsOnIncorrectFields) {
   const Fields input_fields{"chr1", "not a number", "121", "h"};
   EXPECT_THROW(BedRecords::Bed4::fromFields(input_fields), std::exception);
}

TEST(ReferenceMatrixRowParsingTest, BasicFunctionality) {
   const BedRecords::Bed4PlusX expected_parsed_fields{
       1, 1000, 'h', {0.1, 0.1, 0.1}};
   const Fields input_fields{"chr1", "1000", "1001", "h", "10", "10", "10"};
   BedRecords::Bed4PlusX actual_parsed_fields{
       BedRecords::Bed4PlusX::fromFields(input_fields)};
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
   EXPECT_EQ(actual_parsed_fields.start, expected_parsed_fields.start);
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
   EXPECT_EQ(actual_parsed_fields.methylation_proportions,
             expected_parsed_fields.methylation_proportions);
}

TEST(ReferenceMatrixRowParsingTest, ThrowsOnIncorrectFields) {
   const Fields input_fields{"chr1", "1000", "1001", "h", "not a number"};
   EXPECT_THROW(BedRecords::Bed4PlusX::fromFields(input_fields),
                std::exception);
}

TEST(ReferenceMatrixRowParsingTest, ThrowsOnTooFewFields) {
   const Fields input_fields{"chr1", "1000", "1001", "h"};
   EXPECT_THROW(BedRecords::Bed4PlusX::fromFields(input_fields),
                std::out_of_range);
}

TEST(BedMethylRowParsing, BasicFunctionality) {
   const BedRecords::Bed9Plus9 expected_parsed_fields{1, 1000, 'h', 0.1};
   const Fields input_fields{"chr1", "1000", "1001", "h", "100", "10"};
   BedRecords::Bed9Plus9 actual_parsed_fields{
       BedRecords::Bed9Plus9::fromFields(input_fields)};
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
   EXPECT_EQ(actual_parsed_fields.start, expected_parsed_fields.start);
   EXPECT_EQ(actual_parsed_fields.name, expected_parsed_fields.name);
   EXPECT_EQ(actual_parsed_fields.methylation_proportion,
             expected_parsed_fields.methylation_proportion);
}

TEST(BedMethylRowParsing, ThrowsOnIncorrectFields) {
   const Fields input_fields{
       "chr1", "1000", "1001", "h", "100", "not a number"};
   EXPECT_THROW(BedRecords::Bed9Plus9::fromFields(input_fields),
                std::exception);
}

TEST(BedMethylRowParsing, ThrowsOnTooFewFields) {
   const Fields input_fields{"chr1", "1000", "1001", "h", "100"};
   EXPECT_THROW(BedRecords::Bed9Plus9::fromFields(input_fields),
                std::out_of_range);
}

}  // namespace Hylord
