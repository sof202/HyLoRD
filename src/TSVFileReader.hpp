#ifndef TSVFILEREADER_H_
#define TSVFILEREADER_H_

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

template <typename RecordType>
class TSVFileReader {
  private:
   std::string m_file_path{};
   std::vector<RecordType> m_records{};
   std::vector<int> m_columns_to_include{};
   std::function<bool(const std::vector<std::string>&)> m_rowFilter{};
   int m_num_threads{};

   // Memory mapping
   struct stat m_file_info{};
   std::size_t m_file_size{};
   int m_file_descriptor{-1};
   char* m_mapped_data{nullptr};
   void setupMemoryMap();
   void cleanupMemoryMap();

   // Reading
   struct ChunkResult {
      int chunk_index{};
      std::vector<RecordType> records{};
   };
   std::vector<std::string> splitTSVLine(const std::string& line) const;
   const char* findChunkEnd(const char* start, int size) const;
   std::vector<RecordType> processChunk(const char* start,
                                        const char* end) const;
   std::vector<ChunkResult> processChunksOrdered(const char* file_start,
                                                 const char* file_end);

  public:
   TSVFileReader(const std::string_view file_path,
                 const std::vector<int>& columns_to_include = {},
                 std::function<bool(const std::vector<std::string>&)>
                     rowFilter = nullptr,
                 int threads = std::thread::hardware_concurrency()) :
       m_file_path{file_path},
       m_columns_to_include{columns_to_include},
       m_rowFilter{rowFilter},
       m_num_threads{threads} {
      setupMemoryMap();
   }

   void load();
   const std::vector<RecordType>& getRecords() const { return m_records; }

   ~TSVFileReader() { cleanupMemoryMap(); }
};

template <typename RecordType>
inline void TSVFileReader<RecordType>::setupMemoryMap() {
   m_file_descriptor = open(m_file_path.c_str(), O_RDONLY);
   if (m_file_descriptor == -1) {
      throw std::system_error(errno,
                              std::system_category(),
                              "Failed to open file: " + m_file_path);
   }
   if (fstat(m_file_descriptor, &m_file_info) == -1) {
      int fstat_error_number = errno;
      close(m_file_descriptor);
      throw std::system_error(fstat_error_number,
                              std::system_category(),
                              "fstat failed for file: " + m_file_path);
   }
   if (!S_ISREG(m_file_info.st_mode)) {
      close(m_file_descriptor);
      throw std::runtime_error("Not a regular file: " + m_file_path);
   }

   m_file_size = static_cast<std::size_t>(m_file_info.st_size);
   if (m_file_size == 0) {
      close(m_file_descriptor);
      throw std::runtime_error("This file is empty: " + m_file_path);
   }

   m_mapped_data = static_cast<char*>(mmap(
       nullptr, m_file_size, PROT_READ, MAP_PRIVATE, m_file_descriptor, 0));

   if (m_mapped_data == MAP_FAILED) {
      int mmap_error_number = errno;
      close(m_file_descriptor);
      throw std::system_error(mmap_error_number,
                              std::system_category(),
                              "Memory mapping failed for: " + m_file_path);
   }

   madvise(m_mapped_data, m_file_size, MADV_SEQUENTIAL | MADV_WILLNEED);
}

template <typename RecordType>
inline void TSVFileReader<RecordType>::cleanupMemoryMap() {
   if (m_mapped_data) {
      munmap(m_mapped_data, m_file_size);
      m_mapped_data = nullptr;
   }
   if (m_file_descriptor != -1) {
      close(m_file_descriptor);
      m_file_descriptor = -1;
   }
}

template <typename RecordType>
inline std::vector<std::string> TSVFileReader<RecordType>::splitTSVLine(
    const std::string& line) const {
   std::vector<std::string> fields;
   std::size_t start{0};
   std::size_t end{line.find('\t')};

   while (end != std::string::npos) {
      fields.push_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find('\t', start);
   }
   // Final field
   fields.push_back(line.substr(start));

   return fields;
}

template <typename RecordType>
inline std::vector<RecordType> TSVFileReader<RecordType>::processChunk(
    const char* start, const char* end) const {
   std::vector<RecordType> chunk_records;
   const char* line_start{start};

   while (line_start < end) {
      const char* line_end{static_cast<const char*>(
          memchr(line_start, '\n', end - line_start))};
      if (!line_end) line_end = end;

      std::string line(line_start, line_end - line_start);
      std::vector<std::string> fields{splitTSVLine(line)};

      std::vector<std::string> filtered_fields(m_columns_to_include.size());
      for (auto i : m_columns_to_include) {
         if (i < fields.size()) {
            filtered_fields.push_back(fields[i]);
         } else {
            filtered_fields.emplace_back();
         }
      }

      if (!m_rowFilter || m_rowFilter(filtered_fields)) {
         try {
            chunk_records.push_back(RecordType::fromFields(filtered_fields));
         } catch (const std::exception& e) {
            std::cerr << "Record conversion error: " << e.what() << '\n';
         }
      }

      line_start = line_end + 1;
   }
}

#endif
