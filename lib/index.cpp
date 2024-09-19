#include "index.h"

size_t ii::InvertedIndex::Size() const {
  size_t terms_size = terms_.size() * sizeof(std::pair<std::string, size_t>);
  size_t posting_table_size = 0;
  for (int i = 0; i < posting_table_.size(); ++i) {
    posting_table_size +=
        posting_table_[i].size() * sizeof(std::pair<size_t, size_t>);
  }
  size_t position_table_size = position_table_.size() * sizeof(size_t);
  return terms_size + posting_table_size + position_table_size;
}

bool ii::InvertedIndex::Parse(int argc, char** argv) {
  if (argc == 3 && !strcmp(argv[1], "-i")) {
    input_directory_ = argv[2];
    return true;
  }
  return false;
}

std::vector<uint8_t> ii::InvertedIndex::VarintEncoding(size_t n) const {
  std::vector<uint8_t> bytes;
  if (n == 0) {
    bytes.push_back(0);
  }
  while (n > 0) {
    bytes.push_back(n % (1 << 7));
    n >>= 7;
  }
  for (int i = 0; i < bytes.size() - 1; ++i) {
    bytes[i] += 128;
  }
  return bytes;
}

void ii::InvertedIndex::Write(std::ofstream& file, const size_t n) const {
  std::vector<uint8_t> bytes = VarintEncoding(n);
  for (int i = 0; i < bytes.size(); ++i) {
    file << bytes[i];
  }
}

void ii::InvertedIndex::Clear() {
  terms_.clear();
  posting_table_.clear();
  position_table_.clear();
}

void ii::InvertedIndex::Update() {
  std::ofstream term_info(term_info_path, std::ios::binary | std::ios::app);
  std::ofstream posting_table(posting_table_path,
                              std::ios::binary | std::ios::app);
  std::ofstream position_table(position_table_path,
                               std::ios::binary | std::ios::app);
  for (auto term_it = terms_.begin(); term_it != terms_.end(); ++term_it) {
    std::string term = term_it->first;
    size_t ind = term_it->second;

    Write(term_info, term.size());
    term_info << term;
    Write(term_info, posting_table_[ind].size());
    Write(term_info, posting_table.tellp());
    Write(term_info, position_table.tellp());

    size_t prev_DID = 0;

    size_t position_table_ind = 0;
    for (auto posting_it = posting_table_[ind].begin();
         posting_it != posting_table_[ind].end(); ++posting_it) {
      size_t DID = posting_it->first;
      size_t tf = posting_it->second;
      Write(posting_table, DID - prev_DID);
      Write(posting_table, tf);
      prev_DID = DID;
      size_t prev_pos = 0;
      for (int i = 0; i < tf; ++i, ++position_table_ind) {
        Write(position_table, position_table_[ind][position_table_ind] - prev_pos);
        prev_pos = position_table_[ind][position_table_ind];
      }
    }
  }
  term_info.close();
  posting_table.close();
  position_table.close();
  Clear();
}

void ii::InvertedIndex::ParseDocument(const std::string& path) {
  std::ifstream file(path);
  std::string str;
  std::string term;
  size_t line = 0;
  size_t dl = 0;
  size_t DID = N;
  ++N;
  while (std::getline(file, str)) {
    ++line;
    std::stringstream ss(str);
    while (ss >> term) {
      term.erase(std::remove_if(term.begin(), term.end(),
                                [](char c) {
                                  return (c != '-' && c != '_' && ispunct(c));
                                }),
                 term.end());
      std::transform(term.begin(), term.end(), term.begin(), ::tolower);
      if (term.size() != 0 && term != "-" && term != "_") {
        ++dl;
        if (!terms_.contains(term)) {
          terms_[term] = terms_.size();
          std::map<size_t, size_t> m;
          std::vector<size_t> v;
          m[DID] = 1;
          v.emplace_back(line);
          posting_table_.emplace_back(m);
          position_table_.emplace_back(v);
        } else {
          ++posting_table_[terms_[term]][DID];
          position_table_[terms_[term]].emplace_back(line);
        }
      }
      if (Size() >= 32768) {
        Update();
      }
    }
  }
  dl_all += dl;
  std::ofstream doc_info(doc_info_path, std::ios::binary | std::ios::app);
  Write(doc_info, DID);
  Write(doc_info, dl);
  Write(doc_info, path.size());
  doc_info << path;
  doc_info.close();
}

void ii::InvertedIndex::ClearFiles() {
  std::ofstream doc_info(doc_info_path);
  std::ofstream term_info(term_info_path);
  std::ofstream posting_table(posting_table_path);
  std::ofstream position_table(position_table_path);
  doc_info.close();
  term_info.close();
  posting_table.close();
  position_table.close();
}

void ii::InvertedIndex::Launcher(int argc, char** argv) {
  if (!Parse(argc, argv)) {
        std::cerr << "Invalid Arguments\n";
        exit(0);
    }
  ClearFiles();
  for (const auto& file :
       std::filesystem::recursive_directory_iterator(input_directory_)) {
    if (!std::filesystem::is_directory(file)) {
      ParseDocument(file.path().string());
    }
  }
  Update();
  std::ofstream info(info_path, std::ios::binary);
  Write(info, N);
  Write(info, dl_all);
  info.close();
}