#include "io/MemoryMap.hpp"

#include <sys/mman.h>

#include <cerrno>

#include "io/FileDescriptor.hpp"

namespace Hylord::IO {
void MemoryMap::setup(const FileDescriptor& file_descriptor) {
   m_size = file_descriptor.fileSize();
   m_mapped_data = mmap(nullptr,
                        m_size,
                        PROT_READ,
                        MAP_PRIVATE,
                        file_descriptor.fileDescriptor(),
                        0);
   if (m_mapped_data == MAP_FAILED)
      throw std::system_error(
          errno, std::system_category(), "Memory mapping failed");

   madvise(m_mapped_data, m_size, MADV_SEQUENTIAL | MADV_WILLNEED);
}

void MemoryMap::teardown() noexcept {
   if (valid()) {
      munmap(m_mapped_data, m_size);
      m_mapped_data = MAP_FAILED;
      m_size = 0;
   }
}

}  // namespace Hylord::IO
