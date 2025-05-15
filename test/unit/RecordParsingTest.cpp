#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <string_view>

#include "data/BedRecords.hpp"

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

}  // namespace Hylord
