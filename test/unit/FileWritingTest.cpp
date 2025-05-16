#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

#include "HylordException.hpp"
#include "io/writeMetrics.hpp"

namespace Hylord {

class FileWritingTest : public ::testing::Test {
  public:
   std::filesystem::path m_test_dir;
   std::stringstream m_test_buffer;

  protected:
   void SetUp() override {
      m_test_dir = std::filesystem::temp_directory_path() / "test";
      std::filesystem::create_directories(m_test_dir);
   }
   void TearDown() override { std::filesystem::remove_all(m_test_dir); }
};

TEST_F(FileWritingTest, BasicFunctionality) {
   m_test_buffer << "test string";
   std::filesystem::path file_path{m_test_dir / "basic_writing.txt"};
   IO::writeToFile(m_test_buffer, file_path);

   ASSERT_TRUE(std::filesystem::exists(file_path));

   std::ifstream test_file(file_path);
   std::string test_file_contents((std::istreambuf_iterator<char>(test_file)),
                                  std::istreambuf_iterator<char>());
   EXPECT_EQ(test_file_contents, "test string");
}

TEST_F(FileWritingTest, CreatesParentDirectories) {
   m_test_buffer << "test string";
   std::filesystem::path nested_file_path{m_test_dir / "subdir1" / "subdir2" /
                                          "nested_file.txt"};
   IO::writeToFile(m_test_buffer, nested_file_path);

   ASSERT_TRUE(std::filesystem::exists(nested_file_path));

   std::ifstream test_file(nested_file_path);
   std::string test_file_contents((std::istreambuf_iterator<char>(test_file)),
                                  std::istreambuf_iterator<char>());
   EXPECT_EQ(test_file_contents, "test string");
}

TEST_F(FileWritingTest, HandleExistingFileNames) {
   std::filesystem::path file_path{m_test_dir / "existing_file.txt"};
   m_test_buffer << "test string";
   IO::writeToFile(m_test_buffer, file_path);
   m_test_buffer << "test string";
   IO::writeToFile(m_test_buffer, file_path);
   m_test_buffer << "test string";
   IO::writeToFile(m_test_buffer, file_path);

   std::filesystem::path file_path_copy1{m_test_dir / "existing_file_1.txt"};
   std::filesystem::path file_path_copy2{m_test_dir / "existing_file_2.txt"};
   ASSERT_TRUE(std::filesystem::exists(file_path));
   ASSERT_TRUE(std::filesystem::exists(file_path_copy1));
   ASSERT_TRUE(std::filesystem::exists(file_path_copy2));

   std::ifstream test_file(file_path);
   std::string test_file_contents((std::istreambuf_iterator<char>(test_file)),
                                  std::istreambuf_iterator<char>());
   EXPECT_EQ(test_file_contents, "test string");

   std::ifstream test_file_copy1(file_path_copy1);
   std::string test_file_copy1_contents(
       (std::istreambuf_iterator<char>(test_file_copy1)),
       std::istreambuf_iterator<char>());
   EXPECT_EQ(test_file_copy1_contents, "test string");

   std::ifstream test_file_copy2(file_path_copy2);
   std::string test_file_copy2_contents(
       (std::istreambuf_iterator<char>(test_file_copy2)),
       std::istreambuf_iterator<char>());
   EXPECT_EQ(test_file_copy2_contents, "test string");
}

TEST_F(FileWritingTest, ThrowsOnNoWritePermissions) {
   m_test_buffer << "test string";
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
