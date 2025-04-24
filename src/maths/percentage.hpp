#ifndef HYLORD_PERCENTAGE_H_
#define HYLORD_PERCENTAGE_H_

#include <cmath>
#include <cstdlib>

namespace Hylord::Maths {
inline auto convertToPercent(double decimal_value, int precision = 2)
    -> double {
   constexpr double percentage_base{100.0};
   const double scalingFactor{std::pow(10, precision)};

   double percent{std::round(decimal_value * percentage_base * scalingFactor) /
                  scalingFactor};
   // abs is used here as the decimal value from the deconvolution process may
   // be a very small negative number. Without abs this function would output
   // -0, which is undesirable.
   return std::abs(std::max(percent, 0.0));
}

inline auto convertToProportion(double percent_value) -> double {
   constexpr double percentage_base{0.01};
   return percent_value * percentage_base;
}
}  // namespace Hylord::Maths

#endif
