#ifndef HYLORD_PERCENTAGE_H_
#define HYLORD_PERCENTAGE_H_

/**
 * @file    percentage.hpp
 * @brief   Defines function to switch between proportions and percentages
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <cmath>
#include <cstdlib>

namespace Hylord::Maths {
/**
 * Converts decimal to percentage with rounding.
 *
 * Transforms 0-1 range to 0-100% with:
 * - Configurable decimal places (default 2)
 * - Negative value protection
 * - Non-negative clamping
 */
inline auto convertToPercent(double decimal_value, int precision = 2)
    -> double {
   constexpr double percentage_base{100.0};
   const double scaling_factor{std::pow(10, precision)};

   double percent{
       std::round(decimal_value * percentage_base * scaling_factor) /
       scaling_factor};
   // abs is used here as the decimal value from the deconvolution process may
   // be a very small negative number. Without abs this function would output
   // -0, which is undesirable.
   return std::abs(std::max(percent, 0.0));
}

/**
 * Converts percentage to decimal proportion.
 *
 * Transforms percentage (0-100%) to decimal (0-1)
 */
inline auto convertToProportion(double percent_value) -> double {
   constexpr double percentage_base{0.01};
   return percent_value * percentage_base;
}
}  // namespace Hylord::Maths

#endif
