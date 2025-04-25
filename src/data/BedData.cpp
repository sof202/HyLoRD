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
/**
 * Extracts the methylation proportion values from all records and stores them
 * in a dense Eigen vector. The resulting vector will have the same number of
 * elements as there are records.
 */
auto BedMethylData::getAsEigenVector() const -> Vector {
   Vector methylation_proportions(m_records.size());
   for (RowIndex i{}; i < m_records.size(); ++i) {
      methylation_proportions(i) = m_records[i].methylation_proportion;
   }
   return methylation_proportions;
}

/**
 * For each record in the matrix, appends new methylation proportion values
 * based on record type. Records with name 'm' get values from methylation_cdf,
 * others from hydroxymethylation_cdf.
 */
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

/**
 * Constructs an Eigen matrix from the stored methylation proportions,
 * validating column consistency. Each row in the matrix corresponds to a
 * record's methylation proportions.
 * @throws PreprocessingException if records have inconsistent numbers of
 * methylation proportions.
 */
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
