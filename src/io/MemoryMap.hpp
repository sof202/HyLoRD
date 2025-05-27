#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

#include <sys/mman.h>

#include <cstddef>
#include <utility>

#include "io/FileDescriptor.hpp"
namespace Hylord::IO {
class MemoryMap {
  public:
   explicit MemoryMap(const FileDescriptor& file_descriptor) {
      setup(file_descriptor);
   }
   ~MemoryMap() { teardown(); }
   MemoryMap(const MemoryMap&) = delete;
   auto operator=(const MemoryMap&) -> MemoryMap& = delete;

   MemoryMap(MemoryMap&& other) noexcept :
       m_mapped_data(std::exchange(other.m_mapped_data, MAP_FAILED)),
       m_size(std::exchange(other.m_size, 0)) {}

   auto operator=(MemoryMap&& other) noexcept -> MemoryMap& {
      if (this != &other) {
         teardown();
         m_mapped_data = std::exchange(other.m_mapped_data, nullptr);
         m_size = std::exchange(other.m_size, 0);
      }
      return *this;
   }

   [[nodiscard]] auto valid() const -> bool {
      return m_mapped_data != MAP_FAILED;
   }
   [[nodiscard]] auto data() const -> char* {
      return static_cast<char*>(m_mapped_data);
   }
   [[nodiscard]] auto size() const -> std::size_t { return m_size; }

  private:
   void* m_mapped_data{MAP_FAILED};
   std::size_t m_size{};
   void setup(const FileDescriptor& file_descriptor);
   void teardown() noexcept;
};

}  // namespace Hylord::IO

#endif
