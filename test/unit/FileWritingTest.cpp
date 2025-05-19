#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include "HylordException.hpp"
#include "io/writeMetrics.hpp"

namespace Hylord {

class FileWritingTest : public ::testing::Test {
  protected:
   std::filesystem::path m_test_dir;
   std::stringstream m_test_buffer;
   std::string m_test_string{"test string"};
   void SetUp() override {
      m_test_dir = std::filesystem::temp_directory_path() / "test";
      std::filesystem::create_directories(m_test_dir);
   }
   void TearDown() override { std::filesystem::remove_all(m_test_dir); }

   static void writeWrapper(const std::filesystem::path& file_path,
                            const std::string& input_text) {
      std::stringstream buffer;
      buffer << input_text;

      IO::writeToFile(buffer, file_path);
   }
   static void testSuccessfulWrite(const std::filesystem::path& file_path,
                                   const std::string& input_text) {
      ASSERT_TRUE(std::filesystem::exists(file_path));

      std::ifstream file(file_path);
      std::string contents((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
      EXPECT_EQ(contents, input_text);
   }
};

TEST_F(FileWritingTest, BasicFunctionality) {
   writeWrapper(m_test_dir / "basic_writing.txt", m_test_string);
   testSuccessfulWrite(m_test_dir / "basic_writing.txt", m_test_string);
}

TEST_F(FileWritingTest, CreatesParentDirectories) {
   writeWrapper(m_test_dir / "subdir1" / "subdir2" / "basic_writing.txt",
                m_test_string);
   testSuccessfulWrite(
       m_test_dir / "subdir1" / "subdir2" / "basic_writing.txt",
       m_test_string);
}

TEST_F(FileWritingTest, HandleExistingFileNames) {
   writeWrapper(m_test_dir / "existing_file.txt", m_test_string);
   writeWrapper(m_test_dir / "existing_file.txt", m_test_string);
   writeWrapper(m_test_dir / "existing_file.txt", m_test_string);
   testSuccessfulWrite(m_test_dir / "existing_file.txt", m_test_string);
   testSuccessfulWrite(m_test_dir / "existing_file_1.txt", m_test_string);
   testSuccessfulWrite(m_test_dir / "existing_file_2.txt", m_test_string);
}

TEST_F(FileWritingTest, ThrowsOnNoWritePermissions) {
   m_test_buffer << m_test_string;
   std::filesystem::path no_write_dir{m_test_dir / "no_write_dir"};
   std::filesystem::create_directories(no_write_dir);
   std::filesystem::permissions(no_write_dir,
                                std::filesystem::perms::owner_write,
                                std::filesystem::perm_options::remove);
   std::filesystem::path file_path{no_write_dir / "test_file.txt"};
   EXPECT_THROW(IO::writeToFile(m_test_buffer, file_path), FileWriteException);

   // Need to restore permissions to allow cleanup of files
   std::filesystem::permissions(no_write_dir,
                                std::filesystem::perms::owner_write,
                                std::filesystem::perm_options::add);
}

TEST_F(FileWritingTest, ThrowsOnEmptyStringBuffer) {
   std::filesystem::path file_path{m_test_dir / "test_file.txt"};
   EXPECT_THROW(IO::writeToFile(m_test_buffer, file_path), FileWriteException);
}

TEST_F(FileWritingTest, FailOnPathBeingDirectory) {
   EXPECT_THROW(IO::writeToFile(m_test_buffer, m_test_dir),
                FileWriteException);
}

}  // namespace Hylord
