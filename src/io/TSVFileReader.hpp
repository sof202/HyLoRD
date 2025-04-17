#ifndef TSVFILEREADER_H_
#define TSVFILEREADER_H_

/**
 * @file    TSVFileReader.hpp
 * @brief   Parser Implementations for reading tsv files (BED files)
 * @copyright 2025 Sam Fletcher. Licensed under the MIT License. (See LICENSE
 * file in the repository root or https://mit-license.org)
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <future>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "HylordException.hpp"
#include "concepts.hpp"
#include "types.hpp"

namespace Hylord::IO {
/**
 * @class TSVFileReader
 * @brief A thread-safe TSV (Tab-Separated Values) file reader with
 * memory-mapped file support.
 *
 * @tparam RecordType The type of record to parse from the TSV file. Must be
 * compatible with TSVRecord concept.
 *
 * This class provides efficient reading of TSV files with the following
 * features:
 * - Memory-mapped file I/O for high performance
 * - Multi-threaded parsing
 * - Column filtering
 * - Row filtering
 * - Move semantics for efficient resource transfer
 *
 * The reader loads the entire file into memory (via memory mapping) and
 * processes it in parallel chunks.
 *
 * @note This class is not copyable but supports move operations.
 * @note The file must exist and be accessible at construction time.
 *
 * Example usage:
 * @code
 * TSVFileReader<MyRecord> reader("data.tsv", {0, 2, 3}, [](const auto& fields)
 * { return !fields[0].empty(); // Filter out rows with empty first column });
 * reader.load();
 * assert(reader.isLoaded())
 * const auto& records = reader.getRecords();
 * @endcode
 */
template <Records::TSVRecord RecordType>
class TSVFileReader {
  public:
   /**
    * @brief Constructs a TSVFileReader with the given parameters.
    *
    * @param file_path Path to the TSV file to read.
    * @param columns_to_include Indices of columns to include (empty to include
    * all fields)
    * @param rowFilter Optional filter function to exclude rows (nullptr to
    * include all rows).
    * @param threads Number of threads to use for processing (defaults to
    * hardware concurrency).
    */
   TSVFileReader(
       const std::string_view file_path,
       ColumnIndexes columns_to_include = {},
       RowFilter rowFilter = nullptr,
       int threads = static_cast<int>(std::thread::hardware_concurrency())) :
       m_file_path{file_path},
       m_columns_to_include{std::move(columns_to_include)},
       m_rowFilter{std::move(rowFilter)},
       m_num_threads{threads} {}

   TSVFileReader(const TSVFileReader&) = delete;
   auto operator=(const TSVFileReader&) -> TSVFileReader& = delete;

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

   auto operator=(TSVFileReader&& other) noexcept -> TSVFileReader& {
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

   /**
    * @brief Loads and processes the TSV file.
    *
    * This method:
    * 1. Memory-maps the file
    * 2. Divides it into chunks
    * 3. Processes chunks in parallel
    * 4. Combines results
    *
    * @throw std::runtime_error if the file cannot be loaded or parsed.
    */
   void load();
   auto isLoaded() const noexcept -> bool { return m_loaded; }

   using Records = Records::Collection<RecordType>;
   auto extractRecords() -> Records {
      if (!isLoaded()) throw std::runtime_error("No data loaded.");
      return std::move(m_records);
   }

   ~TSVFileReader() { cleanupMemoryMap(); }

  private:
   std::string m_file_path;
   Records m_records{};
   ColumnIndexes m_columns_to_include;
   RowFilter m_rowFilter;
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
   auto splitTSVLine(const std::string& line) const -> Fields;
   auto findChunkEnd(const char* start, int size) const -> const char*;
   auto processChunk(const char* start, const char* end) const -> Records;

   using ChunkResults = std::vector<ChunkResult>;
   auto processFile(const char* file_start, const char* file_end)
       -> ChunkResults;

   // error catching (thread safe)
   mutable std::mutex m_warning_mutex;
   mutable std::vector<std::string> m_warning_messages;
   const int m_max_warning_messages{5};
};

template <Records::TSVRecord RecordType>
inline void TSVFileReader<RecordType>::setupMemoryMap() {
   m_file_descriptor = open(m_file_path.c_str(), O_RDONLY);
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
      throw FileReadException(m_file_path, "Not a regular file");
   }

   m_file_size = static_cast<std::size_t>(m_file_info.st_size);
   if (m_file_size == 0) {
      close(m_file_descriptor);
      throw FileReadException(m_file_path, "File is empty.");
   }

   m_mapped_data = static_cast<char*>(mmap(
       nullptr, m_file_size, PROT_READ, MAP_PRIVATE, m_file_descriptor, 0));

   if (m_mapped_data == MAP_FAILED) {
      int mmap_error_number = errno;
      close(m_file_descriptor);
      throw std::system_error(
          mmap_error_number, std::system_category(), "Memory mapping failed");
   }

   madvise(m_mapped_data, m_file_size, MADV_SEQUENTIAL | MADV_WILLNEED);
}

template <Records::TSVRecord RecordType>
inline void TSVFileReader<RecordType>::cleanupMemoryMap() {
   if (m_mapped_data != nullptr) {
      munmap(m_mapped_data, m_file_size);
      m_mapped_data = nullptr;
   }
   if (m_file_descriptor != -1) {
      close(m_file_descriptor);
      m_file_descriptor = -1;
   }
}

template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::splitTSVLine(
    const std::string& line) const -> Fields {
   Fields fields;
   std::size_t start{0};
   std::size_t end{line.find_first_of("\t ")};

   while (end != std::string::npos) {
      fields.push_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find_first_of("\t ", start);
   }
   // Final field
   fields.push_back(line.substr(start));

   return fields;
}

template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::findChunkEnd(const char* start,
                                                    int size) const -> const
    char* {
   const char* approximate_end{start + size};
   const char* file_end{m_mapped_data + m_file_size};

   if (approximate_end >= file_end) return file_end;

   const char* end{static_cast<const char*>(
       memchr(approximate_end,
              '\n',
              static_cast<std::size_t>(file_end - approximate_end)))};

   return (end != nullptr) ? end : file_end;
}

template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::processChunk(const char* start,
                                                    const char* end) const
    -> std::vector<RecordType> {
   Records chunk_records;
   const char* line_start{start};

   while (line_start < end) {
      const char* line_end{static_cast<const char*>(memchr(
          line_start, '\n', static_cast<std::size_t>(end - line_start)))};
      if (line_end == nullptr) line_end = end;

      std::string line(line_start,
                       static_cast<std::size_t>(line_end - line_start));
      Fields fields{splitTSVLine(line)};

      Fields filtered_fields{};
      filtered_fields.reserve(fields.size());
      if (m_columns_to_include.empty()) {
         filtered_fields = std::move(fields);
      } else {
         for (auto i : m_columns_to_include) {
            if (i < fields.size()) filtered_fields.push_back(fields[i]);
         }
      }

      try {
         if (!m_rowFilter || m_rowFilter(filtered_fields)) {
            chunk_records.push_back(RecordType::fromFields(filtered_fields));
         }
      } catch (const std::exception& e) {
         std::lock_guard<std::mutex> lock(m_warning_mutex);
         std::ostringstream oss;
         oss << "Record conversion warning: " << e.what() << '\n';
         if (line.empty()) {
            oss << "Line was empty.\n";
         } else {
            oss << line << '\n';
         }
         m_warning_messages.emplace_back(oss.str());
      }
      line_start = line_end + 1;
   }
   return chunk_records;
}

template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::processFile(const char* file_start,
                                                   const char* file_end) ->
    typename TSVFileReader<RecordType>::ChunkResults {
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

   ChunkResults chunk_results(chunk_ranges.size());
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

template <Records::TSVRecord RecordType>
void TSVFileReader<RecordType>::load() {
   if (m_loaded) {
      throw HylordException("File is already loaded.");
   }
   try {
      setupMemoryMap();
      const char* file_start{m_mapped_data};
      const char* file_end{m_mapped_data + m_file_size};

      auto chunk_results{processFile(file_start, file_end)};

      // Performance enhancement, we don't know how long a line is going to
      // be, but this is a nice conservative estimate that isn't too large.
      // (based off of BED9+9)
      const std::size_t approximate_line_length{50};
      m_records.reserve(m_file_size / approximate_line_length);

      // Insert chunks in the correct order
      for (auto& result : chunk_results) {
         m_records.insert(m_records.end(),
                          std::make_move_iterator(result.records.begin()),
                          std::make_move_iterator(result.records.end()));
      }
      m_loaded = true;

      if (!m_warning_messages.empty()) {
         int num_warning_messages{static_cast<int>(m_warning_messages.size())};
         std::cerr << "===\n"
                   << num_warning_messages << " warning"
                   << (num_warning_messages > 1 ? "s" : "")
                   << " occurred whilst processing '" << m_file_path << "'.\n";
         for (int i{};
              i < std::min(m_max_warning_messages, num_warning_messages);
              ++i) {
            std::cerr << m_warning_messages[static_cast<std::size_t>(i)]
                      << '\n';
         }
         std::cerr << "These lines will be skipped.\n";
         int remaining_messages{num_warning_messages - m_max_warning_messages};
         if (remaining_messages > 0) {
            std::cerr << remaining_messages << " warning message"
                      << (remaining_messages > 1 ? "s were" : " was")
                      << " surpressed.\n"
                      << "===\n";
         }
      }
   } catch (const std::system_error& e) {
      cleanupMemoryMap();
      throw FileReadException(m_file_path,
                              "Caught system_error with code " +
                                  std::to_string(e.code().value()) + " [" +
                                  e.what() + "].");
   } catch (...) {
      cleanupMemoryMap();
      throw;
   }
   cleanupMemoryMap();
}
}  // namespace Hylord::IO

#endif
