#ifndef WRITE_METRICS_H_
#define WRITE_METRICS_H_

#include "cli.hpp"
#include "core/hylord.hpp"

namespace Hylord::IO {
void writeMetrics(const CMD::HylordConfig& config,
                  const Deconvolver& deconvolver);
}  // namespace Hylord::IO

#endif
