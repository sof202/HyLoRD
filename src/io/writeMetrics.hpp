#ifndef WRITE_METRICS_H_
#define WRITE_METRICS_H_

#include <string_view>
#include <vector>

#include "cli.hpp"
#include "core/hylord.hpp"
#include "types.hpp"

namespace Hylord::IO {
struct CellType {
   std::string cell_type;
   static CellType fromFields(const Fields& fields) {
      return CellType{fields[0]};
   }
};

double convertToPercent(double d, int precision = 2);

std::vector<CellType> generateCellTypeList(
    const std::string_view cell_type_list_file,
    const Deconvolver& deconvolver);

void writeMetrics(const std::string_view output_file_path,
                  const CMD::HylordConfig& config,
                  const Deconvolver& deconvolver);
}  // namespace Hylord::IO

#endif
