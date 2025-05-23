#ifndef HYLORDRNG_H_
#define HYLORDRNG_H_

/**
 * @file    rng.hpp
 * @brief   Defines random number generation for filling out reference matrix
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <algorithm>
#include <iterator>
#include <random>
#include <vector>

#include "pcg_random.hpp"
#include "types.hpp"

/// RNG for HyLoRD to generate reference profiles for novel cell types
namespace Hylord::RNG {
/**
 * Creates a PCG32 pseudo-random number generator seeded from hardware entropy.
 */
inline auto generate() -> pcg32 {
   pcg_extras::seed_seq_from<std::random_device> seed_source;
   pcg32 rng(seed_source);
   return rng;
}
inline pcg32 rng{generate()};

/**
 * Discrete CDF approximating the bimodal distribution of CpG methylation rates
 * in ONT data.
 *
 * Empirical cumulative distribution function modeling observed
 * bimodal methylation patterns, with peaks near 0% and 100% methylation. Uses
 * discrete values for efficient random sampling. Values represent the 0%, 10%,
 * 20%,
 * ..., 100% quantiles of the observed distribution.
 */
static const inline CDF methylation_cdf{0.06884382,
                                        0.10354818,
                                        0.12962329,
                                        0.16059704,
                                        0.20894288,
                                        0.27983389,
                                        0.38286741,
                                        0.53027698,
                                        0.76769743,
                                        0.97110349,
                                        1};
/**
 * Discrete CDF modeling hydroxymethylation distribution in non-neuronal cell
 * types.
 *
 * Empirical cumulative distribution function for hydroxymethylation levels,
 * excluding the distinct neuronal pattern. Represents quantized observations
 * from 0% to 100% in 10% increments for efficient sampling.
 * Values reflect the heavier right-tailed distribution typical in non-neuronal
 cells.
 */
static const inline CDF hydroxymethylation_cdf{0.23067502,
                                               0.57876935,
                                               0.79139396,
                                               0.90436016,
                                               0.96756705,
                                               0.99265250,
                                               0.99879729,
                                               0.99962567,
                                               0.99974549,
                                               0.99975449,
                                               1};

/**
 * Samples a random value from a given cumulative distribution function (CDF).
 *
 * Generates a random value distributed according to the provided CDF using
 * inverse transform sampling. Uses binary search for efficient lookup and
 * handles edge cases from floating-point rounding. Returns a value in [0,1]
 * corresponding to the CDF's quantile spacing.
 */
inline auto getRandomValueFromCDF(const CDF& cdf) -> double {
   const double random_value{
       std::uniform_real_distribution<double>(0.0, 1.0)(rng)};
   auto cdf_lower_bound{std::ranges::lower_bound(cdf, random_value)};
   auto sampled_index{std::distance(cdf.begin(), cdf_lower_bound)};

   // In case of rounding errors
   sampled_index = std::min(sampled_index, std::ssize(cdf) - 1);
   return static_cast<double>(sampled_index) /
          static_cast<double>(cdf.size() - 1);
}
}  // namespace Hylord::RNG

#endif
