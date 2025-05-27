#ifndef FILE_DESCRIPTOR_H_
#define FILE_DESCRIPTOR_H_

#include <fcntl.h>
#include <unistd.h>

#include <cstddef>
#include <filesystem>
#include <utility>

namespace Hylord::IO {
class FileDescriptor {
  public:
   explicit FileDescriptor(const std::filesystem::path& file_path) {
      setup(file_path);
   }
   ~FileDescriptor() { teardown(); }
   FileDescriptor(const FileDescriptor&) = delete;
   auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;

   FileDescriptor(FileDescriptor&& other) noexcept :
       m_file_descriptor(std::exchange(other.m_file_descriptor, -1)),
       m_file_info(other.m_file_info),
       m_file_size(other.m_file_size) {}

   auto operator=(FileDescriptor&& other) noexcept -> FileDescriptor& {
      if (this != &other) {
         teardown();
         m_file_descriptor = std::exchange(other.m_file_descriptor, -1);
         m_file_info = other.m_file_info;
         m_file_size = other.m_file_size;
      }
      return *this;
   }

   [[nodiscard]] auto valid() const -> bool { return m_file_descriptor != -1; }
   [[nodiscard]] auto fileDescriptor() const -> int {
      return m_file_descriptor;
   }
   [[nodiscard]] auto fileSize() const -> std::size_t { return m_file_size; }
   [[nodiscard]] auto fileInfo() const -> const struct stat& {
      return m_file_info;
   }

  private:
   int m_file_descriptor{-1};
   struct stat m_file_info{};
   std::size_t m_file_size{};
   void setup(const std::filesystem::path& file_path);
   void teardown();
};

}  // namespace Hylord::IO

#endif
