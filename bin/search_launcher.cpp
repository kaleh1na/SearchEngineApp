#include <iostream>

#include "lib/index.h"
#include "lib/search.h"

using namespace sse;

int main(int argc, char** argv) {
  SimpleSearchEngine e;
  std::string request;
  size_t n;
  std::cin >> n;
  std::getline(std::cin, request);
  for (int i = 0; i < n; ++i) {
    std::getline(std::cin, request);
    size_t k;
    try {
      k = std::stoull(request);
    } catch (const std::out_of_range& e) {
      std::cerr << "Overflow" << std::endl;
      return 0;
    } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument" << std::endl;
      return 0;
    }
    std::getline(std::cin, request);
    e.Request(request, k);
  }
}