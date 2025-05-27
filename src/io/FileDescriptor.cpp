#include "io/FileDescriptor.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>

#include "HylordException.hpp"

namespace Hylord::IO {
void FileDescriptor::setup(const std::filesystem::path& file_path) {
   m_file_descriptor = open(file_path.c_str(), O_RDONLY);
   if (m_file_descriptor == -1) {
      throw std::system_error(
          errno, std::system_category(), "Failed to open file");
   }
   if (fstat(m_file_descriptor, &m_file_info) == -1) {
      int fstat_error_number = errno;
      close(m_file_descriptor);
      throw std::system_error(
          fstat_error_number, std::system_category(), "fstat failed for file");
   }
   if (!S_ISREG(m_file_info.st_mode)) {
      close(m_file_descriptor);
      throw FileReadException(file_path, "Not a regular file");
   }

   m_file_size = static_cast<std::size_t>(m_file_info.st_size);
   if (m_file_size == 0) {
      close(m_file_descriptor);
      throw FileReadException(file_path, "File is empty.");
   }
}

void FileDescriptor::teardown() {
   if (valid()) {
      close(m_file_descriptor);
      m_file_descriptor = -1;
   }
}

}  // namespace Hylord::IO
