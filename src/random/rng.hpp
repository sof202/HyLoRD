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

namespace Hylord::RNG {
inline pcg32 generate() {
   pcg_extras::seed_seq_from<std::random_device> seed_source;
   pcg32 rng(seed_source);
   return rng;
}
inline pcg32 rng{generate()};

// From previous ONT data collected, cpg methylation rates show the expected
// bimodal distribution with peaks around 0 and 1 (0% and 100%). The mapping
// below recreates that bimodal distribution as a discrete distribution. This
// is done as it is quicker to generate random numbers.
static inline std::vector<double> methylation_cdf{0.06884382,
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
// Hydoxymethylation is similar, but shows a massively different distrubtion
// in neurons than other cell types. We stick with the distributions seen in
// other cell types seen.
static inline std::vector<double> hydroxymethylation_cdf{0.23067502,
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
inline double getRandomMethylation() {
   double r{std::uniform_real_distribution<double>(0.0, 1.0)(rng)};
   auto it{
       std::lower_bound(methylation_cdf.begin(), methylation_cdf.end(), r)};
   auto index{std::distance(methylation_cdf.begin(), it)};

   // In case of rounding errors
   index = std::min(index, std::ssize(methylation_cdf) - 1);
   return static_cast<double>(index) / (methylation_cdf.size() - 1);
}
inline double getRandomHydroxymethylation() {
   double r{std::uniform_real_distribution<double>(0.0, 1.0)(rng)};
   auto it{std::lower_bound(
       hydroxymethylation_cdf.begin(), hydroxymethylation_cdf.end(), r)};
   auto index{std::distance(hydroxymethylation_cdf.begin(), it)};

   // In case of rounding errors
   index = std::min(index, std::ssize(hydroxymethylation_cdf) - 1);
   return static_cast<double>(index) / (hydroxymethylation_cdf.size() - 1);
}

}  // namespace Hylord::RNG

#endif
