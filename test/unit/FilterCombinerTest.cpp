#include <gtest/gtest.h>

#include "cli.hpp"
#include "data/Filters.hpp"
#include "types.hpp"

namespace Hylord {
class FilterCombinerTest : public ::testing::Test {
  protected:
   // min read depth: 10, max read depth: 100, extract only methylation
   const CMD::HylordConfig test_config_methylation{
       .num_threads = 0,
       .cpg_list_file = "",
       .reference_matrix_file = "",
       .cell_type_list_file = "",
       .additional_cell_types = 0,
       .out_file_path = "",
       .max_iterations = 0,
       .convergence_threshold = 0.0,
       .bedmethyl_file = "",
       .min_read_depth = 10,
       .max_read_depth = 100,
       .use_only_methylation_signal = true,
       .use_only_hydroxy_signal = false};
   // min read depth: 10, max read depth: 100, extract only methylation
   const CMD::HylordConfig test_config_hydroxymethylation{
       .num_threads = 0,
       .cpg_list_file = "",
       .reference_matrix_file = "",
       .cell_type_list_file = "",
       .additional_cell_types = 0,
       .out_file_path = "",
       .max_iterations = 0,
       .convergence_threshold = 0.0,
       .bedmethyl_file = "",
       .min_read_depth = 10,
       .max_read_depth = 100,
       .use_only_methylation_signal = false,
       .use_only_hydroxy_signal = true};
};

TEST_F(FilterCombinerTest, NameFiltering) {
   IO::RowFilter methylation_filter{
       Filters::generateNameFilter(test_config_methylation)};
   IO::RowFilter hydroxymethylation_filter{
       Filters::generateNameFilter(test_config_hydroxymethylation)};
   const Fields methylated_row{"chr1", "1000", "1001", "m"};
   const Fields hydroxymethylated_row{"chr1", "1000", "1001", "h"};
   EXPECT_TRUE(methylation_filter(methylated_row));
   EXPECT_FALSE(methylation_filter(hydroxymethylated_row));
   EXPECT_TRUE(hydroxymethylation_filter(hydroxymethylated_row));
   EXPECT_FALSE(hydroxymethylation_filter(methylated_row));
}

}  // namespace Hylord
