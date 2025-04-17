#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Declares the main Hylord run function.
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "cli.hpp"

namespace Hylord {
int run(CMD::HylordConfig& config);
}  // namespace Hylord

#endif
