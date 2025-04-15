#ifndef HYLORD_H_
#define HYLORD_H_

/**
 * @file    hylord.hpp
 * @brief   Contains methods for the main function of hylord
 * @license MIT (See LICENSE file in the repository root)
 */

#include <string_view>

#include "Eigen/Dense"
#include "cli.hpp"
#include "data/BedData.hpp"
#include "io/TSVFileReader.hpp"
#include "qpmad/solver.h"
#include "types.hpp"

namespace Hylord {

template <typename BedFile, typename BedType>
BedFile readFile(const std::string_view file_name,
                 int threads,
                 const IO::ColumnIndexes& fields_to_extract = {},
                 IO::RowFilter rowFilter = nullptr) {
   if (file_name.empty()) return BedFile{};

   return BedFile{[&]() {
      IO::TSVFileReader<BedType> reader{
          file_name, fields_to_extract, rowFilter, threads};
      reader.load();
      return reader.extractRecords();
   }()};
}

class Deconvolver {
  public:
   Deconvolver(int num_cell_types, const Vector& bulk_profile) :
       m_num_cell_types(num_cell_types),
       m_bulk_profile{bulk_profile} {
      initialise();
   }

   qpmad::Solver::ReturnStatus runQpmad(const Matrix& reference);
   Vector cell_proportions() { return m_cell_proportions; }

  private:
   void initialise() {
      m_proportions_lower_bound = Vector::Zero(m_num_cell_types);
      m_proportions_upper_bound = Vector::Ones(m_num_cell_types);
      m_sum_lower_bound = Vector::Ones(1);
      m_sum_upper_bound = Vector::Ones(1);
      m_inequality_matrix = Vector::Ones(m_num_cell_types).transpose();
   }

   int m_num_cell_types;
   Vector m_cell_proportions;         // x
   Vector m_proportions_lower_bound;  // lb
   Vector m_proportions_upper_bound;  // ub
   Vector m_sum_lower_bound;          // Alb
   Vector m_sum_upper_bound;          // Aub
   Matrix m_inequality_matrix;        // A
   Vector m_bulk_profile;             // h
};

/**
 * @brief Subsets input data to common CpGs and adds random additional cell
 * type data
 */
void preprocessInputData(const BedData::BedMethylData& bedmethyl,
                         const BedData::ReferenceMatrixData& reference_matrix,
                         const BedData::CpGData& cpg_list,
                         int additional_cell_types);

/**
 * @brief Update process for unknown reference profiles (see docs for more
 * info).
 */
void update_reference_matrix(Eigen::Ref<Matrix> reference_matrix,
                             const Vector& cell_proportions,
                             const Vector& bulk_profile,
                             int additional_cell_types);

int run(CMD::HylordConfig& config);

}  // namespace Hylord

#endif
