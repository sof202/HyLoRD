/**
 * @file    BedData.cpp
 * @brief   Defines methods for bed data storage container classes
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include "data/BedData.hpp"

#include "Eigen/Dense"
#include "HylordException.hpp"
#include "random/rng.hpp"
#include "types.hpp"

namespace Hylord::BedData {
auto BedMethylData::getAsEigenVector() const -> Vector {
   Vector methylation_proportions(m_records.size());
   for (RowIndex i{}; i < m_records.size(); ++i) {
      methylation_proportions(i) = m_records[i].methylation_proportion;
   }
   return methylation_proportions;
}

void ReferenceMatrixData::addMoreCellTypes(int num_cell_types) {
   for (auto& row : m_records) {
      if (row.name == 'm') {
         for (int i{}; i < num_cell_types; ++i) {
            row.methylation_proportions.emplace_back(
                RNG::getRandomValueFromCDF(RNG::methylation_cdf));
         }
      } else {
         for (int i{}; i < num_cell_types; ++i) {
            row.methylation_proportions.emplace_back(
                RNG::getRandomValueFromCDF(RNG::hydroxymethylation_cdf));
         }
      }
   }
}

auto ReferenceMatrixData::getAsEigenMatrix() const -> Matrix {
   const std::size_t rows = m_records.size();
   const std::size_t cols = m_records[0].methylation_proportions.size();

   for (const auto& record : m_records) {
      if (record.methylation_proportions.size() != cols) {
         throw PreprocessingException(
             "Eigen Matrix Conversion",
             "Inconsistent number of entries in reference matrix.");
      }
   }
   Matrix reference_matrix(rows, cols);
   for (RowIndex i{}; i < rows; ++i) {
      reference_matrix.row(i) =
          Eigen::Map<const Vector>(m_records[i].methylation_proportions.data(),
                                   static_cast<Eigen::Index>(cols));
   }
   return reference_matrix;
}
}  // namespace Hylord::BedData
