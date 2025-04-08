#ifndef TSVFILEREADER_H_
#define TSVFILEREADER_H_

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using Fields = std::vector<std::string>;
using RowFilterFunction = std::function<bool(const std::vector<std::string>&)>;
using ColumnIndexes = std::vector<std::size_t>;

template <typename T>
concept TSVRecord = requires(const Fields& fields) {
   requires std::is_same_v<decltype(T::fromFields(fields)), T>;
   requires !noexcept(T::fromFields(fields));
   requires !std::is_member_function_pointer_v<decltype(&T::fromFields)>;
};

template <TSVRecord RecordType>
class TSVFileReader {
  public:
   TSVFileReader(
       const std::string_view file_path,
       const ColumnIndexes& columns_to_include = {},
       RowFilterFunction rowFilter = nullptr,
       int threads = static_cast<int>(std::thread::hardware_concurrency())) :
       m_file_path{file_path},
       m_columns_to_include{columns_to_include},
       m_rowFilter{rowFilter},
       m_num_threads{threads} {}

   TSVFileReader(const TSVFileReader&) = delete;
   TSVFileReader& operator=(const TSVFileReader&) = delete;

   TSVFileReader(TSVFileReader&& other) noexcept :
       m_file_path{std::move(other.m_file_path)},
       m_records{std::move(other.m_records)},
       m_columns_to_include{std::move(other.m_columns_to_include)},
       m_rowFilter{std::move(other.m_rowFilter)},
       m_num_threads{other.m_num_threads},
       m_file_info{other.m_file_info},
       m_file_size{other.m_file_size},
       m_file_descriptor{std::exchange(other.m_file_descriptor, -1)},
       m_mapped_data{std::exchange(other.m_mapped_data, nullptr)} {}

   TSVFileReader& operator=(TSVFileReader&& other) noexcept {
      if (this != &other) {
         cleanupMemoryMap();

         m_file_path = std::move(other.m_file_path);
         m_records = std::move(other.m_records);
         m_columns_to_include = std::move(other.m_columns_to_include);
         m_rowFilter = std::move(other.m_rowFilter);
         m_num_threads = other.m_num_threads;
         m_file_info = other.m_file_info;
         m_file_size = other.m_file_size;
         m_file_descriptor = std::exchange(other.m_file_descriptor, -1);
         m_mapped_data = std::exchange(other.m_mapped_data, nullptr);
      }
      return *this;
   }

   void load();
   bool isLoaded() const noexcept { return m_loaded; }

   using Records = std::vector<RecordType>;
   const Records& getRecords() const {
      if (!m_loaded) throw std::runtime_error("No data loaded.");
      return m_records;
   }

   ~TSVFileReader() { cleanupMemoryMap(); }

  private:
   std::string m_file_path{};
   Records m_records{};
   ColumnIndexes m_columns_to_include{};
   RowFilterFunction m_rowFilter{};
   int m_num_threads{};
   bool m_loaded{false};

   // Memory mapping
   struct stat m_file_info{};
   std::size_t m_file_size{};
   int m_file_descriptor{-1};
   char* m_mapped_data{nullptr};
   void setupMemoryMap();
   void cleanupMemoryMap();

   // Reading
   struct ChunkResult {
      std::size_t chunk_index{};
      Records records{};
   };
   Fields splitTSVLine(const std::string& line) const;
   const char* findChunkEnd(const char* start, int size) const;
   Records processChunk(const char* start, const char* end) const;

   using ChunkResultVector = std::vector<ChunkResult>;
   ChunkResultVector processFile(const char* file_start, const char* file_end);
};

template <TSVRecord RecordType>
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

template <TSVRecord RecordType>
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

template <TSVRecord RecordType>
inline Fields TSVFileReader<RecordType>::splitTSVLine(
    const std::string& line) const {
   Fields fields;
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

template <TSVRecord RecordType>
inline const char* TSVFileReader<RecordType>::findChunkEnd(const char* start,
                                                           int size) const {
   const char* approximate_end{start + size};
   const char* file_end{m_mapped_data + m_file_size};

   if (approximate_end >= file_end) return file_end;

   const char* end{static_cast<const char*>(
       memchr(approximate_end,
              '\n',
              static_cast<std::size_t>(file_end - approximate_end)))};

   return end ? end : file_end;
}

template <TSVRecord RecordType>
inline std::vector<RecordType> TSVFileReader<RecordType>::processChunk(
    const char* start, const char* end) const {
   Records chunk_records;
   const char* line_start{start};

   while (line_start < end) {
      const char* line_end{static_cast<const char*>(memchr(
          line_start, '\n', static_cast<std::size_t>(end - line_start)))};
      if (!line_end) line_end = end;

      std::string line(line_start,
                       static_cast<std::size_t>(line_end - line_start));
      Fields fields{splitTSVLine(line)};

      Fields filtered_fields{};
      filtered_fields.reserve(fields.size());
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
   return chunk_records;
}

template <TSVRecord RecordType>
inline typename TSVFileReader<RecordType>::ChunkResultVector
TSVFileReader<RecordType>::processFile(const char* file_start,
                                       const char* file_end) {
   std::vector<std::pair<const char*, const char*>> chunk_ranges{};
   int chunk_size = static_cast<int>(m_file_size) / m_num_threads;
   const char* chunk_start = file_start;

   for (int i{0}; i < m_num_threads; ++i) {
      const char* chunk_end{(i == m_num_threads - 1)
                                ? file_end
                                : findChunkEnd(chunk_start, chunk_size)};
      chunk_ranges.emplace_back(chunk_start, chunk_end);
      chunk_start = chunk_end + 1;
   }

   // Parallel processing of chunks
   std::vector<std::future<ChunkResult>> futures;
   for (std::size_t i = 0; i < chunk_ranges.size(); ++i) {
      futures.push_back(
          std::async(std::launch::async, [this, i, &chunk_ranges]() {
             auto records =
                 processChunk(chunk_ranges[i].first, chunk_ranges[i].second);
             return ChunkResult{i, std::move(records)};
          }));
   }

   ChunkResultVector chunk_results(chunk_ranges.size());
   for (auto& future : futures) {
      try {
         auto result = future.get();
         chunk_results[result.chunk_index] = std::move(result);
      } catch (const std::exception& e) {
         std::cerr << "Some chunk could not be processed: " << e.what()
                   << '\n';
      }
   }

   return chunk_results;
}

template <TSVRecord RecordType>
void TSVFileReader<RecordType>::load() {
   if (m_loaded) {
      throw std::runtime_error("File is already loaded.");
   }
   setupMemoryMap();
   try {
      const char* file_start{m_mapped_data};
      const char* file_end{m_mapped_data + m_file_size};

      auto chunk_results{processFile(file_start, file_end)};

      m_records.reserve(m_file_size / 100);

      // Insert chunks in the correct order
      for (auto& result : chunk_results) {
         m_records.insert(m_records.end(),
                          std::make_move_iterator(result.records.begin()),
                          std::make_move_iterator(result.records.end()));
      }
      m_loaded = true;
   } catch (...) {
      cleanupMemoryMap();
      throw;
   }
   cleanupMemoryMap();
}

#endif
