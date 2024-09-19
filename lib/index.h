#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace ii {

class InvertedIndex {
  std::string input_directory_;

  size_t dl_all = 0;
  size_t N = 0;

  std::map<std::string, size_t> terms_;
  std::vector<std::map<size_t, size_t>> posting_table_;
  std::vector<std::vector<size_t>> position_table_;

  const std::string doc_info_path = "info/doc.bin";
  const std::string term_info_path = "info/term.bin";
  const std::string posting_table_path = "info/posting_table.bin";
  const std::string position_table_path = "info/position_table.bin";
  const std::string info_path = "info/info.bin";

  void Write(std::ofstream& file, const size_t n) const;

  void Clear();

  void Update();

  void ParseDocument(const std::string& path);

  void ClearFiles();

  bool Parse(int argc, char** argv);

 public:
  size_t Size() const;

  std::vector<uint8_t> VarintEncoding(size_t n) const;

  void Launcher(int argc, char** argv);
};

}  // namespace ii
