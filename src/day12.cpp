#include <charconv>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
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

// Assume c to be 'a' - 'z'
int to_int(char c) { return static_cast<int>(c - 'a') + 1; }

std::vector<std::vector<int>> parse_input(const std::vector<std::string> &in) {
  return in | ranges::views::transform([](auto line) {
           return line | ranges::views::transform([](auto c) {
                    if (c == 'S') {
                      return 1;
                    } else if (c == 'E') {
                      return 26;
                    } else {
                      return to_int(c);
                    }
                  }) |
                  ranges::to<std::vector>;
         }) |
         ranges::to<std::vector>;
}

std::pair<int, int> find(const std::vector<std::string> &map, char c) {
  for (auto [i, l] : map | ranges::views::enumerate) {
    auto it = ranges::find(l, c);
    if (it != ranges::end(l)) {
      return {i, ranges::distance(ranges::begin(l), it)};
    }
  }
  return {-1, -1};
}

std::pair<int, int> find_start(const std::vector<std::string> &map) {
  return find(map, 'S');
}

std::pair<int, int> find_end(const std::vector<std::string> &map) {
  return find(map, 'E');
}

using Index = std::pair<int, int>;
using Something = std::pair<Index, int>;

constexpr int infinity = 100'000'000;

auto init_dijkstra(const std::vector<std::vector<int>> &map,
                   bool part2 = false) {
  int rows = map.size();
  int cols = map[0].size();

  auto coords = ranges::views::cartesian_product(ranges::views::iota(0, rows),
                                                 ranges::views::iota(0, cols));
  std::map<std::pair<int, int>, int> distances;

  auto cmp = [](auto left, auto right) { return left.second > right.second; };
  std::priority_queue<Something, std::vector<Something>, decltype(cmp)>
      unvisited(cmp);

  for (auto coord : coords) {
    auto pair = std::make_pair(std::get<0>(coord), std::get<1>(coord));
    if (part2 && map[pair.first][pair.second] == 1) {
      distances.emplace(pair, 0);
      unvisited.emplace(pair, 0);
    } else {
      distances.emplace(pair, infinity);
    }
  }

  return std::make_pair(unvisited, distances);
}

int dijkstra(const std::vector<std::vector<int>> &map, Index end,
             std::optional<Index> start = {}) {
  auto in_bounds = [&](std::pair<int, int> idx) {
    int rows = map.size();
    int cols = map[0].size();

    return idx.first >= 0 && idx.first < rows && idx.second >= 0 &&
           idx.second < cols;
  };

  auto tmp = init_dijkstra(map, !start.has_value());
  auto unvisited = tmp.first;
  auto distances = tmp.second;

  if (start.has_value()) {
    distances[*start] = 0;
  }

  unvisited.emplace(*start, distances[*start]);

  std::map<std::pair<int, int>, std::pair<int, int>> previous;

  auto update = [&map, &unvisited, &distances, &previous,
                 in_bounds](auto v, auto u, auto distance) {
    if (in_bounds(v)) {
      auto vheight = map[v.first][v.second];
      auto uheight = map[u.first][u.second];
      auto new_distance = distance + 1;

      if (vheight <= uheight + 1) {
        // If we haven't been there yet, visit it next
        if (distances[v] == infinity) {
          unvisited.emplace(v, new_distance);
        }

        // Update, if the new distance to v is lower from u, than the previous
        // one
        if (new_distance < distances[v]) {
          distances[v] = new_distance;
          previous[v] = u;
        }
      }
    }
  };

  while (!unvisited.empty()) {
    auto u = unvisited.top();
    unvisited.pop();

    // Visit neighbours of u, this could be nicer, but what ever
    auto v = u.first;
    ++v.second;
    update(v, u.first, u.second);

    v = u.first;
    --v.second;
    update(v, u.first, u.second);

    v = u.first;
    ++v.first;
    update(v, u.first, u.second);

    v = u.first;
    --v.first;
    update(v, u.first, u.second);
  }

  return distances[end];
}

void part1(const std::vector<std::string> &in) {
  auto map = parse_input(in);

  auto start = find_start(in);
  auto end = find_end(in);

  auto shortest_path = dijkstra(map, end, start);
  fmt::print("Distance to end: {}\n", shortest_path);
}

void part2(const std::vector<std::string> &in) {
  auto map = parse_input(in);

  auto end = find_end(in);

  auto shortest_path = dijkstra(map, end);
  fmt::print("Distance to end: {}\n", shortest_path);
}

int main() {
  auto in = input();

  part1(in);
  part2(in);

  return 0;
}
