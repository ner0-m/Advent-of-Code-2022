#include <iostream>
#include <string>
#include <variant>
#include <vector>

#define FMT_HEADER_ONLY = 1
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <range/v3/all.hpp>

#include "input.hpp"

std::size_t first_all_different(const std::string s, int range) {
  auto all_different =
      s | ranges::views::sliding(range) | ranges::views::transform([](auto x) {
        // all are different, if the sorted and uniqued size is the same as
        // original
        auto size = ranges::size(x);
        auto unique = std::move(x | ranges::to<std::vector>) |
                      ranges::actions::sort | ranges::actions::unique;
        return size == ranges::size(unique);
      });

  // find the first where all chars are different one, it's lazy so no extra
  // computation
  auto first_different = ranges::find(all_different, true);

  auto first = ranges::begin(all_different);
  return ranges::distance(first, first_different) + range;
}

void part1(const std::string &in) {
  int range = 4;
  auto pos = first_all_different(in, range);

  fmt::print("Processed characters till first 4 different: {}, {}\n", pos,
             in.substr(pos - range - 1, range));
}

void part2(const std::string &in) {
  int range = 14;
  auto pos = first_all_different(in, range);

  fmt::print("Processed characters till first 4 different: {}, {}\n", pos,
             in.substr(pos - range - 1, range));
}

int main() {
  auto in = input();
  part1(in.front());
  part2(in.front());
}
