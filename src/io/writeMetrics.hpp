#ifndef WRITE_METRICS_H_
#define WRITE_METRICS_H_

/**
 * @file    writeMetrics.hpp
 * @brief   Declares write function for the outputs of the deconvolution
 * algorithm
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "cli.hpp"
#include "core/Deconvolver.hpp"

namespace Hylord::IO {
/// Writes deconvolution results to stdout or file (given by user).
void writeMetrics(const CMD::HylordConfig& config,
                  const Deconvolution::Deconvolver& deconvolver);

void writeToFile(const std::stringstream& buffer,
                 const std::filesystem::path& out_path);
}  // namespace Hylord::IO

#endif
