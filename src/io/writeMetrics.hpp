#ifndef WRITE_METRICS_H_
#define WRITE_METRICS_H_

#include "cli.hpp"
#include "core/Deconvolver.hpp"

namespace Hylord::IO {
void writeMetrics(const CMD::HylordConfig& config,
                  const Deconvolution::Deconvolver& deconvolver);
}  // namespace Hylord::IO

#endif
