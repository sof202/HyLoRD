#ifndef HYLORDTYPEDEFS_H_
#define HYLORDTYPEDEFS_H_

/**
 * @file    types.hpp
 * @brief   Defines all typedefs used in HyLoRD
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "Eigen/Dense"

namespace Hylord {
using RowIndex = std::ptrdiff_t;
using RowIndexes = std::vector<RowIndex>;
using Fields = std::vector<std::string>;
using Vector = Eigen::VectorXd;
using Matrix = Eigen::MatrixXd;

namespace IO {
using RowFilter = std::function<bool(const Fields&)>;
using ColumnIndexes = std::vector<std::size_t>;
using MapRange = std::pair<const char*, const char*>;
}  // namespace IO

namespace RNG {
using CDF = std::vector<double>;
}  // namespace RNG
}  // namespace Hylord

#endif
