#ifndef HYLORDRNG_H_
#define HYLORDRNG_H_

#include <map>
#include <random>

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
static inline std::map<int, double> methylation_mapping{{1, 0.0},
                                                        {2, 0.0408},
                                                        {3, 0.1209},
                                                        {4, 0.2},
                                                        {5, 0.3},
                                                        {6, 0.5},
                                                        {7, 0.6},
                                                        {8, 0.85},
                                                        {9, 1}};
// Hydoxymethylation is similar, but shows a massively different distrubtion
// in neurons than other cell types. We stick with the distributions seen in
// other cell types seen.
static inline std::map<int, double> hydroxymethylation_mapping{{1, 0.0},
                                                               {2, 0.0},
                                                               {3, 0.0},
                                                               {4, 0.0},
                                                               {5, 0.1},
                                                               {6, 0.1},
                                                               {7, 0.1},
                                                               {8, 0.2},
                                                               {9, 0.4}};
inline double get_random_methylation() {
   return methylation_mapping[std::uniform_int_distribution<int>(0, 10)(rng)];
}
inline double get_random_hydroxymethylation() {
   return hydroxymethylation_mapping[std::uniform_int_distribution<int>(
       1, 9)(rng)];
}

}  // namespace Hylord::RNG

#endif
