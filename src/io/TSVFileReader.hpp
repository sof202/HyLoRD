#ifndef TSVFILEREADER_H_
#define TSVFILEREADER_H_

/**
 * @file    TSVFileReader.hpp
 * @brief  Parser Implementations for reading tsv files (BED files)
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
#include <filesystem>
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
#include "io/FileDescriptor.hpp"
#include "io/MemoryMap.hpp"
#include "types.hpp"

/// Defines Input and Output methods for HyLoRD
namespace Hylord::IO {
/**
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
 * TSVFileReader<MyRecord> reader{
 *   "data.tsv",
 *   {0, 2, 3},
 *   [](const auto& fields) { return !fields[0].empty();}
 * };
 * reader.load();
 * assert(reader.isLoaded())
 * const auto& records = reader.getRecords();
 * @endcode
 */
template <Records::TSVRecord RecordType>
class TSVFileReader {
  public:
   /**
    * Constructs a TSVFileReader with the given parameters.
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
       std::filesystem::path file_path,
       ColumnIndexes columns_to_include = {},
       RowFilter rowFilter = nullptr,
       int threads = static_cast<int>(std::thread::hardware_concurrency())) :
       m_file_path{std::move(file_path)},
       m_columns_to_include{std::move(columns_to_include)},
       m_row_filter{std::move(rowFilter)},
       m_num_threads{std::max(1, threads)} {}

   TSVFileReader(const TSVFileReader&) = delete;
   auto operator=(const TSVFileReader&) -> TSVFileReader& = delete;

   TSVFileReader(TSVFileReader&& other) noexcept = default;
   auto operator=(TSVFileReader&& other) noexcept -> TSVFileReader& = default;

   /// Loads and processes the TSV file.
   void load();
   auto isLoaded() const noexcept -> bool { return m_loaded; }

   using Records = Records::Collection<RecordType>;
   /**
    * Extracts and returns all loaded records
    * @throws std::runtime_error if no data has been loaded
    */
   auto extractRecords() -> Records {
      if (!isLoaded()) throw std::runtime_error("No data loaded.");
      return std::move(m_records);
   }

   ~TSVFileReader() = default;

  private:
   std::filesystem::path m_file_path;
   Records m_records{};
   ColumnIndexes m_columns_to_include;
   RowFilter m_row_filter;
   int m_num_threads{};
   bool m_loaded{false};

   // Memory mapping
   FileDescriptor m_file_descriptor{m_file_path};
   MemoryMap m_memory_map{m_file_descriptor};
   /// Get the start and end pointers of the file
   auto mappedRange() const -> MapRange;

   // Reading
   struct ChunkResult {
      std::size_t chunk_index{};
      Records records{};
   };
   /// Splits a TSV line into individual fields.
   auto splitTSVLine(const std::string& line) const -> Fields;
   /// Finds the end of a chunk for parallel processing.
   auto findChunkEnd(const char* start, int size) const -> const char*;
   /// Processes a chunk of TSV data into records.
   auto processChunk(MapRange map_range) -> Records;

   using ChunkResults = std::vector<ChunkResult>;
   /// Processes TSV file in parallel chunks
   auto processFile(MapRange map_range) -> ChunkResults;

   // error catching (thread safe)
   mutable std::mutex m_warning_mutex;
   mutable std::vector<std::string> m_warning_messages;
   static constexpr int m_max_warning_messages{5};
   int m_number_of_warning_messages{};
};

template <Records::TSVRecord RecordType>
auto TSVFileReader<RecordType>::mappedRange() const -> MapRange {
   if (!m_memory_map.valid())
      throw FileReadException(m_file_path, "No valid memory mapping.");
   return {.start = m_memory_map.data(),
           .end = m_memory_map.data() + m_file_descriptor.fileSize()};
}

/**
 * Parses tab or space delimited fields from a line and returns them
 * as a vector. Handles both tabs and spaces as delimiters and includes the
 * final field. Spaces are required due to the silly format of bedmethyl files.
 */
template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::splitTSVLine(
    const std::string& line) const -> Fields {
   Fields fields;
   std::size_t start{0};
   std::size_t end{line.find_first_of("\t ")};

   while (end != std::string::npos) {
      fields.emplace_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find_first_of("\t ", start);
   }
   // Final field
   fields.emplace_back(line.substr(start));

   return fields;
}

/**
 * Locates the nearest newline character after the approximate chunk
 * end to ensure complete records in each chunk. Returns file end if no newline
 * found.
 */
template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::findChunkEnd(const char* start,
                                                    int size) const -> const
    char* {
   const char* approximate_end{start + size};
   const char* file_end{m_memory_map.data() + m_file_descriptor.fileSize()};

   if (approximate_end >= file_end) return file_end;

   const char* end{static_cast<const char*>(
       memchr(approximate_end,
              '\n',
              static_cast<std::size_t>(file_end - approximate_end)))};

   return (end != nullptr) ? end : file_end;
}

/**
 * Parses each line in the chunk, applies column filtering if
 * specified, and converts valid lines into record objects. Invalid records
 * generate warnings while valid ones are added to the result vector.
 * Thread-safe warning collection is implemented due to parallel processing.
 */
template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::processChunk(MapRange map_range)
    -> std::vector<RecordType> {
   Records chunk_records;
   const char* line_start{map_range.start};

   while (line_start < map_range.end) {
      const char* line_end{static_cast<const char*>(
          memchr(line_start,
                 '\n',
                 static_cast<std::size_t>(map_range.end - line_start)))};
      if (line_end == nullptr) line_end = map_range.end;

      std::string line(line_start,
                       static_cast<std::size_t>(line_end - line_start));
      Fields fields{splitTSVLine(line)};

      Fields filtered_fields{};
      filtered_fields.reserve(fields.size());
      if (m_columns_to_include.empty()) {
         filtered_fields = std::move(fields);
      } else {
         for (auto column : m_columns_to_include) {
            if (column < fields.size())
               filtered_fields.push_back(fields[column]);
         }
      }

      try {
         if (!m_row_filter || m_row_filter(filtered_fields)) {
            chunk_records.emplace_back(
                RecordType::fromFields(filtered_fields));
         }
      } catch (const std::exception& e) {
         if (std::ssize(m_warning_messages) < m_max_warning_messages) {
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
         m_number_of_warning_messages++;
      }
      line_start = line_end + 1;
   }
   return chunk_records;
}

/**
 * Divides file into chunks by thread count, finds line boundaries,
 * and processes concurrently. Manages threads, tasks and results
 * while preserving order. Handles per-chunk exceptions gracefully.
 */
template <Records::TSVRecord RecordType>
inline auto TSVFileReader<RecordType>::processFile(MapRange map_range) ->
    typename TSVFileReader<RecordType>::ChunkResults {
   std::vector<std::pair<const char*, const char*>> chunk_ranges{};
   int chunk_size{static_cast<int>(m_file_descriptor.fileSize()) /
                  m_num_threads};
   const char* chunk_start{map_range.start};
   const char* file_end{map_range.end};

   for (int i{0}; i < m_num_threads; ++i) {
      const char* chunk_end{(i == m_num_threads - 1)
                                ? file_end
                                : findChunkEnd(chunk_start, chunk_size)};
      chunk_ranges.emplace_back(chunk_start, chunk_end);
      chunk_start = chunk_end + 1;
   }

   // Parallel processing of chunks
   std::vector<std::future<ChunkResult>> futures;
   for (std::size_t i{}; i < chunk_ranges.size(); ++i) {
      futures.push_back(
          std::async(std::launch::async, [this, i, &chunk_ranges]() {
             std::vector<RecordType> records{processChunk(
                 {chunk_ranges[i].first, chunk_ranges[i].second})};
             return ChunkResult{i, std::move(records)};
          }));
   }

   ChunkResults chunk_results(chunk_ranges.size());
   for (auto& future : futures) {
      try {
         auto result{future.get()};
         chunk_results[result.chunk_index] = std::move(result);
      } catch (const std::exception& e) {
         std::cerr << "Some chunk could not be processed: " << e.what()
                   << '\n';
      }
   }

   return chunk_results;
}

/**
 * This method:
 * 1. Divides memory map into chunks
 * 2. Processes chunks in parallel
 * 3. Combines results
 *
 * @throw HylordException if the file is already loaded.
 * @throw FileReadException if the file cannot be loaded or parsed.
 */
template <Records::TSVRecord RecordType>
void TSVFileReader<RecordType>::load() {
   if (m_loaded) {
      throw HylordException("File is already loaded.");
   }
   try {
      auto chunk_results{processFile(mappedRange())};

      // Performance enhancement, we don't know how long a line is going to
      // be, but this is a nice conservative estimate that isn't too large.
      // (based off of BED9+9)
      const std::size_t approximate_line_length{50};
      m_records.reserve(m_file_descriptor.fileSize() /
                        approximate_line_length);

      // Insert chunks in the correct order
      for (auto& result : chunk_results) {
         m_records.insert(m_records.end(),
                          std::make_move_iterator(result.records.begin()),
                          std::make_move_iterator(result.records.end()));
      }
      m_loaded = true;

      if (m_number_of_warning_messages != 0) {
         std::cerr << "===\n"
                   << m_number_of_warning_messages << " warning"
                   << (m_number_of_warning_messages > 1 ? "s" : "")
                   << " occurred whilst processing '" << m_file_path << "'.\n";
         for (int i{}; i < std::min(m_max_warning_messages,
                                    m_number_of_warning_messages);
              ++i) {
            std::cerr << m_warning_messages[static_cast<std::size_t>(i)]
                      << '\n';
         }
         std::cerr << "These lines will be skipped.\n";
         int remaining_messages{m_number_of_warning_messages -
                                m_max_warning_messages};
         if (remaining_messages > 0) {
            std::cerr << remaining_messages << " warning message"
                      << (remaining_messages > 1 ? "s were" : " was")
                      << " surpressed.\n"
                      << "===\n";
         }
      }
   } catch (const std::system_error& e) {
      throw FileReadException(m_file_path,
                              "Caught system_error with code " +
                                  std::to_string(e.code().value()) + " [" +
                                  e.what() + "].");
   }
}
}  // namespace Hylord::IO

#endif
