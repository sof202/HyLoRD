#ifndef TSVFILEREADER_H_
#define TSVFILEREADER_H_

#include <sys/stat.h>

#include <functional>
#include <string>
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
   int m_file_size{};
   int m_file_descriptor{-1};
   char* m_mapped_data{nullptr};
   void setupMemoryMap();
   void cleanupMemoryMap();

   // Reading
   struct ChunkResult {
      int chunk_index{};
      std::vector<RecordType> records{};
   };
   const char* findChunkEnd(const char* start, int size) const;
   std::vector<std::string> splitTSVLine(const std::string& line) const;
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

#endif
