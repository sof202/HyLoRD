#include <gtest/gtest.h>

#include <cmath>

#include "maths/percentage.hpp"

namespace Hylord {
TEST(PercentageConversionTest, BasicFunctionality) {
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0), 0.0);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(1), 100.0);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.5), 50.0);
}

TEST(PercentageConversionTest, PrecisionHandling) {
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.5555, 0), 56.0);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.5555, 1), 55.6);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.5555, 2), 55.55);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.5555, 3), 55.550);
}

TEST(PercentageConversionTest, NegativeInputClampingToZero) {
   const double tiny_negative_proportion{-1e-8};
   double percentage{Maths::convertToPercent(tiny_negative_proportion)};
   EXPECT_TRUE(percentage == 0.0 && !(std::signbit(percentage)))
       << "Expected +0.0, but instead got " << percentage << '\n';
}

TEST(PercentageConversionTest, EdgeCases) {
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(1e-10, 8), 1e-8);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.4999, 0), 50.0);
   EXPECT_DOUBLE_EQ(Maths::convertToPercent(0.4949, 0), 49.0);
}

TEST(ProportionConversionTest, BasicFunctionality) {
   EXPECT_DOUBLE_EQ(Maths::convertToProportion(50.0), 0.5);
   EXPECT_DOUBLE_EQ(Maths::convertToProportion(100.0), 1.0);
   EXPECT_DOUBLE_EQ(Maths::convertToProportion(0.0), 0.0);
}
}  // namespace Hylord
