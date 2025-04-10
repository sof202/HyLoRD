#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include <string_view>
#include <vector>

namespace Hylord {
using RowIndexVector = std::vector<std::size_t>;

template <typename BedTypeOne, typename BedTypeTwo>
std::pair<RowIndexVector, RowIndexVector> findOverLappingIndexes(
    const BedTypeOne& bed_one, const BedTypeTwo& bed_two);

void run(const std::string_view bedmethyl_file,
         const std::string_view reference_matrix_file,
         const std::string_view cpg_list_file,
         const std::string_view cell_type_list_file,
         const int additional_cell_types,
         const int threads);

}  // namespace Hylord

#endif
