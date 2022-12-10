#include <charconv>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#define FMT_HEADER_ONLY = 1
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <range/v3/all.hpp>

#include "input.hpp"
#include "overloaded.hpp"

struct Up {
  int distance;
};
struct Down {
  int distance;
};
struct Left {
  int distance;
};
struct Right {
  int distance;
};

using Direction = std::variant<std::monostate, Up, Down, Left, Right>;

std::vector<Direction> to_directions(const std::vector<std::string> &in) {
  std::vector<Direction> dirs;
  dirs.reserve(in.size());

  for (auto i : in) {
    auto dist = std::stoi(std::string(i.begin() + 2, i.end()));
    if (i[0] == 'U') {
      dirs.push_back(Up{dist});
    } else if (i[0] == 'D') {
      dirs.push_back(Down{dist});
    } else if (i[0] == 'L') {
      dirs.push_back(Left{dist});
    } else if (i[0] == 'R') {
      dirs.push_back(Right{dist});
    } else {
      fmt::print("ERROR: {}\n", i);
    }
  }
  return dirs;
}

using Index = std::pair<int, int>;

auto move_once(Direction d, Index idx) {
  std::visit(overloaded{[&](Right) { ++idx.first; }, [&](Left) { --idx.first; },
                        [&](Up) { ++idx.second; }, [&](Down) { --idx.second; },
                        [](auto) {}},
             d);
  return idx;
}

auto istouching(Index head, Index tail) {
  auto d1 = std::abs(tail.first - head.first);
  auto d2 = std::abs(tail.second - head.second);
  return d1 <= 1 && d2 <= 1;
}

auto move_close_to(Index tail, Index head) {
  if (head.first > tail.first) {
    ++tail.first;
  } else if (head.first < tail.first) {
    --tail.first;
  }
  if (head.second > tail.second) {
    ++tail.second;
  } else if (head.second < tail.second) {
    --tail.second;
  }
  return tail;
}

int count_tail_positions(const std::vector<Direction> &dirs, int num_knots) {
  std::map<Index, int> counter;
  std::vector<Index> knots(num_knots, {0, 0});

  for (auto d : dirs) {
    auto distance =
        std::visit(overloaded{[](std::monostate) { return 0; },
                              [](auto dir) { return dir.distance; }},
                   d);

    for ([[maybe_unused]] auto j : ranges::views::ints(0, distance)) {
      knots.front() = move_once(d, knots.front());

      for (auto i : ranges::views::ints(1ul, knots.size())) {
        if (!istouching(knots[i - 1], knots[i])) {
          knots[i] = move_close_to(knots[i], knots[i - 1]);
        }
      }
      counter[knots.back()] = 1;
    }
  }

  return counter.size();
}

void part1(const std::vector<Direction> &dirs) {
  fmt::print("With 2 knots, the tail visits {} positions\n",
             count_tail_positions(dirs, 2));
}

void part2(const std::vector<Direction> &dirs) {
  fmt::print("With 10 knots, the tail visits {} positions\n",
             count_tail_positions(dirs, 10));
}

int main() {
  auto in = input();
  auto dirs = to_directions(in);

  part1(dirs);
  part2(dirs);

  return 0;
}
