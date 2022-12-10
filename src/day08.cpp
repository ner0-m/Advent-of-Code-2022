#include <charconv>
#include <cmath>
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

std::string getcol(const std::string &str, int col, int rows) {
  return str | ranges::views::drop(col) | ranges::views::stride(rows + 1) |
         ranges::to<std::string>;
}

auto num_visible(std::string row) {
  auto ints = row | ranges::views::transform([](auto c) { return c - '0'; });
  auto tmp = ints | ranges::views::partial_sum([](auto x, auto y) {
               return std::max(x, y);
             }) |
             ranges::views::unique | ranges::to<std::vector>;

  auto first = row.begin();
  // fmt::print("{}\n", tmp);
  ranges::for_each(tmp, [&](auto x) {
    auto it = ranges::find(first, row.end(), std::to_string(x)[0]);
    if (it != row.end()) {
      // fmt::print("distance from {}, to {}: {}\n", *first, *it,
      //            ranges::distance(first, it));
      ranges::transform(first, it, first, [](auto c) { return '?'; });
      // fmt::print("{}\n", row);
      first = it + 1;
    }
  });
  // fmt::print("distance from beginning to first: {}\n", );

  return std::make_tuple(row, ranges::distance(row.begin(), first));
}

int to_num(char c) { return static_cast<int>(c - '0'); }

void part1() {
  auto in = input();

  auto rows = in.size();
  auto cols = in.front().size();

  int counter = 0;
  for (auto i = 1; i < rows - 1; ++i) {
    for (auto j = 1; j < cols - 1; ++j) {

      // Is the tree at (i, j) visible from above
      auto m = -1;
      for (int k = 0; k < i; ++k) {
        m = std::max(m, to_num(in[k][j]));
      }

      // Is the tree at (i, j) visible from below
      auto n = -1;
      for (int k = cols - 1; k > i; --k) {
        n = std::max(n, to_num(in[k][j]));
      }

      // Is the tree at (i, j) visible from below
      auto w = -1;
      for (int k = 0; k < j; ++k) {
        w = std::max(w, to_num(in[i][k]));
      }

      // Is the tree at (i, j) visible from below
      auto e = -1;
      for (int k = cols - 1; k > j; --k) {
        e = std::max(e, to_num(in[i][k]));
      }
      // fmt::print("({}, {}): {} {} {} {}\n", i, j, n, m, e, w);
      auto tmp = std::min(n, std::min(m, std::min(e, w)));

      if (tmp < to_num(in[i][j])) {
        ++counter;
      }
    }
  }

  fmt::print("{}\n", counter + (rows * 2) + (cols - 2) * 2);
}

int main() {
  auto in = input();

  auto rows = in.size();
  auto cols = in.front().size();

  int counter = 0;
  for (auto i = 1; i < rows - 1; ++i) {
    for (auto j = 1; j < cols - 1; ++j) {
      auto cur = to_num(in[i][j]);
      // Is the tree at (i, j) visible from above
      int k = i - 1;
      for (; k > 0; --k) {
        auto c = to_num(in[k][j]);
        if (c >= cur) {
          break;
        }
      }

      auto nd = k == 0 ? i : i - k;

      // Is the tree at (i, j) visible from below
      k = i + 1;
      for (; k < rows; ++k) {
        auto c = to_num(in[k][j]);
        if (c >= cur) {
          break;
        }
      }
      auto sd = k == rows ? (int)k - i - 1 : k - i;

      // Is the tree at (i, j) visible from left
      k = j - 1;
      for (; k > 0; --k) {
        auto c = to_num(in[i][k]);
        if (c >= cur) {
          break;
        }
      }
      auto ed = k == 0 ? j : j - k;

      // Is the tree at (i, j) visible from right
      k = j + 1;
      for (; k < cols; ++k) {
        auto c = to_num(in[i][k]);
        if (c >= cur) {
          break;
        }
      }
      auto wd = k == rows ? (int)k - j - 1 : k - j;

      counter = std::max(counter, nd * sd * ed * wd);
      // fmt::print("({}, {}) = {}: {}\n", i, j, cur, nd * sd * ed * wd);
      // auto tmp = std::min(n, std::min(m, std::min(e, w)));
      //
      // if (tmp < to_num(in[i][j])) {
      //   ++counter;
      // }
    }
  }

  // fmt::print("{}\n", rows + rows + cols - 2 + cols - 2);
  // fmt::print("{}\n", counter + (rows * 2) + (cols - 2) * 2);
  fmt::print("{}\n", counter);
  fmt::print("\n");

  // for (auto i : in) {
  //   fmt::print("{}\n", i);
  // }
}
/*
int main() {
  auto in = input();
  auto instr = in | ranges::views::join('\n') | ranges::to<std::string>();
  int rows = in.size();
  int cols = in.front().size();

  auto copy = instr;

  copy = std::move(copy) | ranges::actions::transform([](auto x) {
           if (x == '\n') {
             return x;
           } else {
             return '?';
           }
         });
  // Copy corners
  copy[0] = instr[0];
  copy[rows -1] = instr[rows - 1];
  copy[(rows - 1) * cols] = instr[(rows - 1) * cols];
  copy[rows * cols] = instr[rows * cols];

  // for (int i = 1; i < rows - 1; ++i) {
  //   auto row = in[i];
  //   auto [tmp1, it1] = num_visible(row);
  //
  //   auto rowfirst = copy.begin() + i * (rows + 1);
  //   auto rowlast = copy.begin() + i * (rows + 1) + rows;
  //   std::copy(tmp1.begin(), tmp1.begin() + it1, rowfirst);
  //
  //   auto [tmp2, it2] =
  //       num_visible(row | ranges::views::reverse | ranges::to<std::string>);
  //
  //   std::copy(tmp2.begin(), tmp2.begin() + it2,
  //             std::make_reverse_iterator(rowlast));
  // }

  // fmt::print("{}\n\n", copy);

  // for (int i = 1; i < cols - 1; ++i) {
  //   std::string col = getcol(instr, i, rows);
  //   auto [tmp1, it1] = num_visible(col);
  //
  //   for (auto [i, j] :
  //        ranges::views::ints(0, rows * cols) | ranges::views::drop(i) |
  //            ranges::views::stride(rows + 1) | ranges::views::take(it1) |
  //            ranges::views::enumerate) {
  //     copy[j] = *(tmp1.begin() + i);
  //   }
  //
  //   auto [tmp2, it2] =
  //       num_visible(col | ranges::views::reverse | ranges::to<std::string>);
  //
  //   for (auto [ii, jj] :
  //        ranges::views::ints(0, rows * cols + i) | ranges::views::reverse |
  //            ranges::views::stride(rows + 1) | ranges::views::take(it2) |
  //            ranges::views::enumerate) {
  //     // fmt::print("{}, {}\n", ii, jj);
  //     copy[jj] = *(tmp1.begin() + ii);
  //   }
  // }

  fmt::print("{}\n", copy);
  auto visible =
      ranges::count_if(copy, [](auto c) { return c >= '0' && c <= '9'; });
  fmt::print("{}\n", visible);

  // fmt::print("{}\n", instr);
  // fmt::print("{}, {}\n", visible_rows, visible_cols);
  // fmt::print("{}\n", visible_rows + visible_cols + 4);
}

*/
