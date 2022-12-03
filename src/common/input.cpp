#include "input.hpp"

#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> input(char delim) {
  std::vector<std::string> in;
  std::string line;
  while (std::getline(std::cin, line, delim)) {
    in.push_back(line);
  }
  return in;
}
