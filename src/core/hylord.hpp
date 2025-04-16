#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include "cli.hpp"

namespace Hylord {
int run(CMD::HylordConfig& config);
}  // namespace Hylord

#endif
