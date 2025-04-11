#include "data/BedData.hpp"

#include <stdexcept>

#include "Eigen/Dense"
#include "types.hpp"

namespace Hylord::BedData {
Vector BedMethylData::getAsEigenVector() const {
   Vector methylation_percentages(m_records.size());
   for (RowIndex i{}; i < m_records.size(); ++i) {
      methylation_percentages(i) = m_records[i].methylation_percentage;
   }
   return methylation_percentages;
}

Matrix ReferenceMatrixData::getAsEigenMatrix() const {
   const std::size_t rows = m_records.size();
   const std::size_t cols = m_records[0].methylation_percentages.size();

   for (const auto& record : m_records) {
      if (record.methylation_percentages.size() != cols) {
         throw std::runtime_error(
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
