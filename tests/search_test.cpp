#include "lib/search.h"

#include <gtest/gtest.h>

#include "lib/index.h"

using namespace ii;
using namespace sse;

TEST(SearchTestSuit, RequestCorrectnessTest) {
  SimpleSearchEngine search;
  std::vector<std::string> correct_requests{
      "for",         "apple AND juice",      "(while OR for) AND vector",
      "(((maybe)))", "(hello) OR ((world))", "()",
      "and"};
  std::vector<std::string> uncorrect_requests{
      "for AND",           "for AND OR list", "((apple)",
      "meeting And apple", "hello world",     "AND"};
  for (int i = 0; i < correct_requests.size(); ++i) {
    ASSERT_TRUE(
        search.CheckСorrectness(search.SplitRequest(correct_requests[i])));
  }
  for (int i = 0; i < uncorrect_requests.size(); ++i) {
    ASSERT_FALSE(
        search.CheckСorrectness(search.SplitRequest(uncorrect_requests[i])));
  }
}

TEST(SearchTestSuit, VarintTest) {
  InvertedIndex in;
  std::vector<uint8_t> bytes = in.VarintEncoding(0);
  std::vector<uint8_t> ans = {0};
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  bytes = in.VarintEncoding(19876);
  ans = {164, 155, 1};
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  bytes = in.VarintEncoding(7);
  ans = {7};
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  bytes = in.VarintEncoding(117891023);
  ans = {207, 191, 155, 56};
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
}

TEST(SearchTestSuit, FilesCreatureTest) {
  InvertedIndex(in);
  int argc = 3;
  char** argv = new char*[argc];
  argv[0] = (char*)"build/bin/index_launcher";
  argv[1] = (char*)"-i";
  argv[2] = (char*)"files";
  in.Launcher(argc, argv);
  std::ifstream info("info/info.bin");
  std::ifstream doc("info/doc.bin");
  std::ifstream posi("info/position_table.bin");
  std::ifstream post("info/posting_table.bin");
  std::ifstream term("info/term.bin");
  ASSERT_TRUE(info.is_open());
  ASSERT_TRUE(doc.is_open());
  ASSERT_TRUE(posi.is_open());
  ASSERT_TRUE(post.is_open());
  ASSERT_TRUE(term.is_open());
  delete[] argv;
}

TEST(SearchTestSuit, InfoCodingTest) {
  InvertedIndex(in);
  int argc = 3;
  char** argv = new char*[argc];
  argv[0] = (char*)"build/bin/index_launcher";
  argv[1] = (char*)"-i";
  argv[2] = (char*)"files";
  in.Launcher(argc, argv);
  std::ifstream info("info/info.bin");
  std::vector<uint8_t> bytes;
  std::vector<uint8_t> ans{5, 139, 9};
  while (!info.eof()) {
    uint8_t byte;
    info.read(reinterpret_cast<char*>(&byte), 1);
    if (info.eof()) {
      break;
    }
    bytes.emplace_back(byte);
  }
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  delete[] argv;
}

TEST(SearchTestSuit, TermCodingTest) {
  InvertedIndex(in);
  int argc = 3;
  char** argv = new char*[argc];
  argv[0] = (char*)"build/bin/index_launcher";
  argv[1] = (char*)"-i";
  argv[2] = (char*)"files/test";
  in.Launcher(argc, argv);
  std::ifstream term("info/term.bin");
  std::vector<uint8_t> bytes;
  std::vector<uint8_t> ans{5, 97, 112, 112, 108, 101, 2, 0,
                           0, 3,  108, 111, 108, 1,   4, 4};
  while (!term.eof()) {
    uint8_t byte;
    term.read(reinterpret_cast<char*>(&byte), 1);
    if (term.eof()) {
      break;
    }
    bytes.emplace_back(byte);
  }
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  delete[] argv;
}

TEST(SearchTestSuit, DocCodingTest) {
  InvertedIndex(in);
  int argc = 3;
  char** argv = new char*[argc];
  argv[0] = (char*)"build/bin/index_launcher";
  argv[1] = (char*)"-i";
  argv[2] = (char*)"files/test";
  in.Launcher(argc, argv);
  std::ifstream doc("info/doc.bin");
  std::vector<uint8_t> bytes;
  std::vector<uint8_t> ans{0,   4,   16,  102, 105, 108, 101, 115, 47,  116,
                           101, 115, 116, 47,  51,  46,  116, 120, 116, 1,
                           1,   16,  102, 105, 108, 101, 115, 47,  116, 101,
                           115, 116, 47,  50,  46,  116, 120, 116};
  while (!doc.eof()) {
    uint8_t byte;
    doc.read(reinterpret_cast<char*>(&byte), 1);
    if (doc.eof()) {
      break;
    }
    bytes.emplace_back(byte);
  }
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  delete[] argv;
}

TEST(SearchTestSuit, PositionCodingTest) {
  InvertedIndex(in);
  int argc = 3;
  char** argv = new char*[argc];
  argv[0] = (char*)"build/bin/index_launcher";
  argv[1] = (char*)"-i";
  argv[2] = (char*)"files/test";
  in.Launcher(argc, argv);
  std::ifstream posi("info/position_table.bin");
  std::vector<uint8_t> bytes;
  std::vector<uint8_t> ans{1, 0, 1, 1, 1};
  while (!posi.eof()) {
    uint8_t byte;
    posi.read(reinterpret_cast<char*>(&byte), 1);
    if (posi.eof()) {
      break;
    }
    bytes.emplace_back(byte);
  }
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  delete[] argv;
}

TEST(SearchTestSuit, PostingCodingTest) {
  InvertedIndex(in);
  int argc = 3;
  char** argv = new char*[argc];
  argv[0] = (char*)"build/bin/index_launcher";
  argv[1] = (char*)"-i";
  argv[2] = (char*)"files/test";
  in.Launcher(argc, argv);
  std::ifstream post("info/posting_table.bin");
  std::vector<uint8_t> bytes;
  std::vector<uint8_t> ans{0, 3, 1, 1, 0, 1};
  while (!post.eof()) {
    uint8_t byte;
    post.read(reinterpret_cast<char*>(&byte), 1);
    if (post.eof()) {
      break;
    }
    bytes.emplace_back(byte);
  }
  ASSERT_EQ(ans.size(), bytes.size());
  for (int i = 0; i < ans.size(); ++i) {
    ASSERT_EQ(ans[i], bytes[i]);
  }
  delete[] argv;
}