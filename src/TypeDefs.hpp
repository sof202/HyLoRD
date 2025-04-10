#ifndef HYLORDTYPEDEFS_H_
#define HYLORDTYPEDEFS_H_

#include <functional>
#include <string>
#include <vector>

namespace Hylord {
using RowIndex = std::size_t;
using RowIndexes = std::vector<RowIndex>;
using Fields = std::vector<std::string>;

namespace IO {
using RowFilterFunction = std::function<bool(const std::vector<std::string>&)>;
using ColumnIndexes = std::vector<std::size_t>;
}  // namespace IO
}  // namespace Hylord

#endif
