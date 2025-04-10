#ifndef HYLORDTYPEDEFS_H_
#define HYLORDTYPEDEFS_H_

/**
 * @file    types.hpp
 * @brief   Defines all typedefs used in HyLoRD
 * @license MIT (See LICENSE file in the repository root)
 */

#include <functional>
#include <string>
#include <vector>

#include "Eigen/Dense"

namespace Hylord {
using RowIndex = std::size_t;
using RowIndexes = std::vector<RowIndex>;
using Fields = std::vector<std::string>;
using Vector = Eigen::VectorXd;
using Matrix = Eigen::MatrixXd;

namespace IO {
using RowFilter = std::function<bool(const Fields&)>;
using ColumnIndexes = std::vector<std::size_t>;
}  // namespace IO
}  // namespace Hylord

#endif
