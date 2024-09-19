#pragma once

#include <iostream>

#include "index.h"

namespace sse {

struct DocInfo {
  size_t dl;
  std::string path;
  DocInfo(size_t dl, const std::string& path) : dl(dl), path(path) {}
  DocInfo() = default;
};

struct TermInfo {
  size_t ind;
  size_t df;
  std::vector<size_t> size;
  std::vector<std::pair<size_t, size_t>> pos;
  TermInfo(size_t ind) : ind(ind) {}
  TermInfo() = default;
};

class Node {
 public:
  virtual std::set<size_t> calculate() const = 0;
};

class TermNode : public Node {
 public:
  TermNode(const std::set<size_t>& value) : value(value) {}
  virtual std::set<size_t> calculate() const override { return value; }

 private:
  const std::set<size_t> value;
};

class AndNode : public Node {
 public:
  AndNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
      : left(left), right(right) {}

  virtual std::set<size_t> calculate() const override {
    std::set<size_t> s;
    std::set<size_t> s1 = left->calculate();
    std::set<size_t> s2 = right->calculate();
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                          std::inserter(s, s.begin()));
    return s;
  }

 private:
  std::shared_ptr<Node> left;
  std::shared_ptr<Node> right;
};

class OrNode : public Node {
 public:
  OrNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right)
      : left(left), right(right) {}

  virtual std::set<size_t> calculate() const override {
    std::set<size_t> s;
    std::set<size_t> s1 = left->calculate();
    std::set<size_t> s2 = right->calculate();
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                   std::inserter(s, s.begin()));
    return s;
  }

 private:
  std::shared_ptr<Node> left;
  std::shared_ptr<Node> right;
};

struct PostingIterator {
  using iterator_type = std::map<size_t, size_t>::iterator;
  iterator_type it1;
  iterator_type it2;
  std::string term;

  PostingIterator(iterator_type it1, iterator_type it2, const std::string& term)
      : it1(it1), it2(it2), term(term) {}
};

class SimpleSearchEngine {
 private:
  double N;
  double dl_all;

  const double k = 2;
  const double b = 0.75;

  std::map<size_t, DocInfo> documents_;
  std::map<std::string, TermInfo> terms_;
  std::vector<std::map<size_t, size_t>> posting_table_;
  std::map<size_t, std::vector<size_t>> position_table_;

  const std::string doc_info_path = "info/doc.bin";
  const std::string term_info_path = "info/term.bin";
  const std::string posting_table_path = "info/posting_table.bin";
  const std::string position_table_path = "info/position_table.bin";
  const std::string info_path = "info/info.bin";

  void GetInfo(const std::set<std::string>& words);

  void GetDocs(const std::set<std::size_t>& docs);

  void GetLines(const std::set<size_t>& DID);

  size_t Read(std::ifstream& file) const;

  double FindRelevance(const std::string& term, const size_t DID);

  std::shared_ptr<Node> ParseExpression(
      const std::vector<std::string>& expression, const size_t start,
      const size_t end);

 public:
  bool CheckСorrectness(const std::vector<std::string>& request) const;

  std::vector<std::string> SplitRequest(std::string& request) const;

  void Request(std::string& request, const size_t k);
};

}  // namespace sse
