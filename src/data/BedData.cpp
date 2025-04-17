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
Vector BedMethylData::getAsEigenVector() const {
   Vector methylation_percentages(m_records.size());
   for (RowIndex i{}; i < m_records.size(); ++i) {
      methylation_percentages(i) = m_records[i].methylation_percentage;
   }
   return methylation_percentages;
}

void ReferenceMatrixData::addMoreCellTypes(int num_cell_types) {
   for (auto& row : m_records) {
      if (row.name == 'm') {
         for (int i{}; i < num_cell_types; ++i) {
            row.methylation_percentages.emplace_back(
                RNG::getRandomMethylation());
         }
      } else {
         for (int i{}; i < num_cell_types; ++i) {
            row.methylation_percentages.emplace_back(
                RNG::getRandomHydroxymethylation());
         }
      }
   }
}

Matrix ReferenceMatrixData::getAsEigenMatrix() const {
   const std::size_t rows = m_records.size();
   const std::size_t cols = m_records[0].methylation_percentages.size();

   for (const auto& record : m_records) {
      if (record.methylation_percentages.size() != cols) {
         throw PreprocessingException(
             "Eigen Matrix Conversion",
             "Inconsistent number of entries in reference matrix.");
      }
   }
   Matrix reference_matrix(rows, cols);
   for (RowIndex i{}; i < rows; ++i) {
      reference_matrix.row(i) = Eigen::Map<const Vector>(
          m_records[i].methylation_percentages.data(), cols);
   }
   return reference_matrix;
}
}  // namespace Hylord::BedData
