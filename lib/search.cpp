#include "search.h"

size_t sse::SimpleSearchEngine::Read(std::ifstream& file) const {
  std::vector<uint8_t> bytes;
  uint8_t byte;
  file.read(reinterpret_cast<char*>(&byte), 1);
  bytes.emplace_back(byte);
  while (byte / 128 != 0) {
    file.read(reinterpret_cast<char*>(&byte), 1);
    bytes.emplace_back(byte);
  }
  size_t ans = byte;
  for (int i = bytes.size() - 2; i >= 0; --i) {
    ans *= 128;
    ans += bytes[i] - 128;
  }
  return ans;
}

double sse::SimpleSearchEngine::FindRelevance(const std::string& term,
                                              const size_t DID) {
  double tf = posting_table_[terms_[term].ind][DID];
  double df = terms_[term].df;
  double dl = documents_[DID].dl;
  double dl_avg = (double)dl_all / N;
  return (tf * (k + 1)) / (tf + k * (1 - b + b * (dl / dl_avg))) *
         std::log2(N / df);
}

bool sse::SimpleSearchEngine::CheckСorrectness(
    const std::vector<std::string>& request) const {
  if (request[0] == "AND" || request[0] == "OR") return false;
  if (request[request.size() - 1] == "AND" ||
      request[request.size() - 1] == "OR")
    return false;
  size_t open_bracket = 0;
  size_t close_bracket = 0;
  for (int i = 0; i < request.size() - 1; ++i) {
    std::string cur = request[i];
    std::string next = request[i + 1];
    if (cur == "(") {
      ++open_bracket;
      if (next == "AND" || next == "OR") {
        return false;
      }
    } else if (cur == ")") {
      ++close_bracket;
      if (!(next == "AND" || next == "OR" || next == ")")) {
        return false;
      }
    } else if (cur == "AND" || cur == "OR") {
      if (next == "AND" || next == "OR" || next == ")") {
        return false;
      }
    } else {
      if (!(next == "AND" || next == "OR" || next == ")")) {
        return false;
      }
    }
  }
  if (request[request.size() - 1] == "(") {
    ++open_bracket;
  }
  if (request[request.size() - 1] == ")") {
    ++close_bracket;
  }
  return open_bracket == close_bracket;
}

std::shared_ptr<sse::Node> sse::SimpleSearchEngine::ParseExpression(
    const std::vector<std::string>& expression, const size_t start,
    const size_t end) {
  std::stack<std::shared_ptr<Node>> operands;
  std::stack<std::string> operators;
  for (int i = start; i < end; ++i) {
    if (expression[i] == "(") {
      int j = i + 1;
      int brackets = 1;
      while (brackets > 0) {
        if (expression[j] == "(") {
          brackets++;
        } else if (expression[j] == ")") {
          brackets--;
        }
        j++;
      }
      operands.push(ParseExpression(expression, i + 1, j - 1));
      i = j - 1;
    } else if (expression[i] == "AND" || expression[i] == "OR") {
      operators.push(expression[i]);
    } else if (expression[i] == ")") {
      while (!operators.empty()) {
        std::string op = operators.top();
        operators.pop();
        std::shared_ptr<Node> right = operands.top();
        operands.pop();
        std::shared_ptr<Node> left = operands.top();
        operands.pop();
        if (op == "AND") {
          operands.push(std::make_shared<AndNode>(left, right));
        } else {
          operands.push(std::make_shared<OrNode>(left, right));
        }
      }
    } else {
      std::set<size_t> s;
      std::map<size_t, size_t> posting_list =
          posting_table_[terms_[expression[i]].ind];
      for (auto it = posting_list.begin(); it != posting_list.end(); ++it) {
        s.insert(it->first);
      }
      operands.push(std::make_shared<TermNode>(s));
    }
  }
  while (!operators.empty()) {
    std::string op = operators.top();
    operators.pop();
    std::shared_ptr<Node> right = operands.top();
    operands.pop();
    std::shared_ptr<Node> left = operands.top();
    operands.pop();
    if (op == "AND") {
      operands.push(std::make_shared<AndNode>(left, right));
    } else {
      operands.push(std::make_shared<OrNode>(left, right));
    }
  }
  return operands.top();
}

std::vector<std::string> sse::SimpleSearchEngine::SplitRequest(
    std::string& request) const {
  std::regex reg("\\w+|\\S");
  std::smatch match;
  std::vector<std::string> exp;
  while (std::regex_search(request, match, reg)) {
    std::string token = match.str();
    exp.push_back(token);
    request = match.suffix();
  }
  return exp;
}

void sse::SimpleSearchEngine::GetInfo(const std::set<std::string>& words) {
  std::ifstream term_info(term_info_path, std::ios::binary);
  std::ifstream posting_table(posting_table_path, std::ios::binary);
  while (!term_info.eof()) {
    size_t term_size = Read(term_info);
    if (term_info.eof()) {
      break;
    }
    std::string term = "";
    for (int i = 0; i < term_size; ++i) {
      char byte;
      term_info.read(reinterpret_cast<char*>(&byte), 1);
      term += byte;
    }
    size_t posting_list_size = Read(term_info);
    size_t posting_ind = Read(term_info);
    size_t position_ind = Read(term_info);
    if (words.contains(term)) {
      posting_table.seekg(posting_ind);
      if (!terms_.contains(term)) {
        terms_[term] = TermInfo(terms_.size());
        std::map<size_t, size_t> posting_list;
        size_t prev = 0;
        for (int i = 0; i < posting_list_size; ++i) {
          size_t DID = Read(posting_table) + prev;
          prev = DID;
          size_t tf = Read(posting_table);
          posting_list[DID] = tf;
        }
        posting_table_.push_back(posting_list);
      } else {
        size_t ind = terms_[term].ind;
        size_t prev = 0;
        for (int i = 0; i < posting_list_size; ++i) {
          size_t DID = Read(posting_table) + prev;
          prev = DID;
          size_t tf = Read(posting_table);
          if (!posting_table_[ind].contains(DID)) {
            posting_table_[ind][DID] = tf;
          } else {
            posting_table_[ind][DID] += tf;
          }
        }
      }
      terms_[term].size.emplace_back(posting_list_size);
      terms_[term].pos.emplace_back(std::make_pair(posting_ind, position_ind));
      terms_[term].df = posting_table_[terms_[term].ind].size();
    }
  }
  term_info.close();
  posting_table.close();
}

void sse::SimpleSearchEngine::GetDocs(const std::set<std::size_t>& docs) {
  std::ifstream doc_info(doc_info_path, std::ios::binary);
  while (!doc_info.eof()) {
    size_t DID = Read(doc_info);
    if (doc_info.eof()) {
      break;
    }
    size_t dl = Read(doc_info);
    size_t path_size = Read(doc_info);
    std::string path = "";
    for (int i = 0; i < path_size; ++i) {
      char byte;
      doc_info.read(reinterpret_cast<char*>(&byte), 1);
      path += byte;
    }
    if (docs.contains(DID)) {
      documents_[DID] = DocInfo(dl, path);
    }
  }
}

void sse::SimpleSearchEngine::GetLines(const std::set<size_t>& docs) {
  std::ifstream posting_table(posting_table_path, std::ios::binary);
  std::ifstream position_table(position_table_path, std::ios::binary);
  for (auto it = terms_.begin(); it != terms_.end(); ++it) {
    for (int i = 0; i < it->second.pos.size(); ++i) {
      size_t posting_list_size = it->second.size[i];
      size_t posting_ind = it->second.pos[i].first;
      size_t position_ind = it->second.pos[i].second;
      posting_table.seekg(posting_ind);
      position_table.seekg(position_ind);
      size_t prev_DID = 0;
      for (int j = 0; j < posting_list_size; ++j) {
        size_t DID = Read(posting_table) + prev_DID;
        prev_DID = DID;
        size_t position_list_size = Read(posting_table);
        if (!docs.contains(DID)) {
          for (int k = 0; k < position_list_size; ++k) {
            Read(position_table);
          }
        } else {
          size_t prev_line = 0;
          for (int k = 0; k < position_list_size; ++k) {
            size_t line = Read(position_table) + prev_line;
            prev_line = line;
            position_table_[DID].push_back(line);
          }
        }
      }
    }
  }
  for (auto it = position_table_.begin(); it != position_table_.end(); ++it) {
    std::sort((it->second).begin(), (it->second).end());
  }
  posting_table.close();
  position_table.close();
}

void sse::SimpleSearchEngine::Request(std::string& request, const size_t k) {
  std::ifstream info(info_path, std::ios::binary);
  N = Read(info);
  dl_all = Read(info);
  info.close();
  std::vector<std::string> exp = SplitRequest(request);
  std::set<std::string> words;
  for (int i = 0; i < exp.size(); ++i) {
    if (exp[i] != "{" && exp[i] != "}" && exp[i] != "AND" && exp[i] != "OR") {
      words.insert(exp[i]);
    }
  }
  if (!CheckСorrectness(exp)) {
    std::cerr << "Invalid request\n";
    return;
  }
  GetInfo(words);
  std::set<std::string> correct_words;
  for (auto it = words.begin(); it != words.end(); ++it) {
    if (terms_.contains(*it)) {
      correct_words.insert(*it);
    }
  }
  if (correct_words.size() == 0) {
    std::cout << "No matching files\n";
    return;
  }
  words = correct_words;
  std::shared_ptr<Node> expression = ParseExpression(exp, 0, exp.size());
  std::set<size_t> docs = expression->calculate();
  GetDocs(docs);
  std::vector<PostingIterator> its;
  for (auto i = words.begin(); i != words.end(); ++i) {
    auto it1 = posting_table_[terms_[*i].ind].begin();
    auto it2 = posting_table_[terms_[*i].ind].end();
    while (it1 != it2 && !docs.contains(it1->first)) {
      ++it1;
    }
    if (it1 != it2) {
      its.emplace_back(PostingIterator(it1, it2, *i));
    }
  }
  std::multimap<double, size_t> ans;
  while (!its.empty()) {
    sort(its.begin(), its.end(), [](PostingIterator it1, PostingIterator it2) {
      return it1.it1->first < it2.it1->first;
    });
    size_t DID = its[0].it1->first;
    double rel = 0;
    for (int i = 0; i < its.size(); ++i) {
      if (its[i].it1->first == DID) {
        rel += FindRelevance(its[i].term, DID);
        ++its[i].it1;
        while (its[i].it1 != its[i].it2 && !docs.contains(its[i].it1->first)) {
          ++its[i].it1;
        }
      }
    }
    if (ans.size() < k) {
      ans.insert(std::make_pair(rel, DID));
    } else {
      auto it = ans.begin();
      if (it->first < rel) {
        ans.erase(it);
        ans.insert(std::make_pair(rel, DID));
      }
    }
    its.erase(
        std::remove_if(its.begin(), its.end(),
                       [](PostingIterator it) { return it.it1 == it.it2; }),
        its.end());
  }
  std::set<size_t> DIDs;
  for (auto it = ans.begin(); it != ans.end(); ++it) {
    DIDs.insert(it->second);
  }
  GetLines(DIDs);
  for (auto it = ans.end(); it != ans.begin();) {
    --it;
    std::cout << documents_[it->second].path << ' ';
    for (int i = 0; i != position_table_[it->second].size(); ++i) {
      std::cout << position_table_[it->second][i] << ' ';
    }
    std::cout << '\n';
  }

  documents_.clear();
  terms_.clear();
  posting_table_.clear();
  position_table_.clear();
}