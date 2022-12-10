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

struct Noop {
  int duration = 1;
};

struct Add {
  int inc;
  int duration = 2;
};

using Instruction = std::variant<Noop, Add>;

auto parse(const std::vector<std::string> &in) {
  std::vector<Instruction> instr;
  instr.reserve(in.size());
  for (auto str : in) {
    if (str.starts_with("noop")) {
      instr.push_back(Noop{});
    } else {
      auto split = str | ranges::views::split(' ') | ranges::views::drop(1) |
                   ranges::views::join | ranges::to<std::string>;

      instr.push_back(Add{std::stoi(split)});
    }
  }
  return instr;
}

struct Visitor {
  std::pair<int, int> operator()(Noop noop) const { return {0, noop.duration}; }
  std::pair<int, int> operator()(Add a) const { return {a.inc, a.duration}; }
};

struct InfoVisitor {
  void operator()(Noop noop) const {
    fmt::print("Start Cycle {:>3}: Begin executing noop\n", clock);
  }
  void operator()(Add a) const {
    fmt::print("Start Cycle {:>3}: Begin executing addx {}\n", clock, a.inc);
  }

  int clock;
};

struct EndInfoVisitor {
  void operator()(Noop noop) const {
    fmt::print("End Cycle {:>5}: Finish executing noop\n", clock);
  }
  void operator()(Add a) const {
    fmt::print(
        "End Cycle {:>5}: Finish executing addx {} (Register is now {})\n",
        clock, a.inc, X);
  }

  int clock;
  int X;
};

auto part1(const std::vector<Instruction> &instrs) {
  std::vector<int> strengths;

  auto X = 1;
  auto clock = 1;

  for (auto instr : instrs) {
    auto [inc, instr_time] = std::visit(Visitor{}, instr);

    for ([[maybe_unused]] auto i : ranges::views::ints(0, instr_time)) {
      ++clock;
    }

    // If the instruction time was 2, then we might jump from 19 to 21, and need
    // to calculate the strength before adding the increment
    if (instr_time == 2 && (clock == 21 || (clock - 20) % 40 == 1)) {
      fmt::print("{}: {}, {}, {}\n", clock - 1, X, inc, (clock - 1) * X);
      strengths.push_back((clock - 1) * X);
    }

    X += inc;

    if (clock == 20 || (clock - 20) % 40 == 0) {
      fmt::print("{}: {}, {}, {}\n", clock, X, inc, clock * X);
      strengths.push_back(clock * X);
    }
  }
  fmt::print("{}\n", ranges::accumulate(strengths, 0));
}

void part2(const std::vector<Instruction> &instructions, bool verbose = false) {
  auto X = 1;
  auto clock = 1;

  auto print_sprite = [](auto X) {
    std::string sprite_pos(40, '.');
    if (X > 1) {
      sprite_pos[X - 1] = '#';
    }
    if (X > 0 && X < 40) {
      sprite_pos[X] = '#';
    }
    if (X < 39) {
      sprite_pos[X + 1] = '#';
    }
    fmt::print("Sprite position: {}\n", sprite_pos);
  };

  std::vector<std::string> rows;
  std::string row = "";
  for (auto instr : instructions) {
    auto [inc, instr_time] = std::visit(Visitor{}, instr);

    if (verbose) {
      std::visit(InfoVisitor{clock}, instr);
    }

    for (auto i : ranges::views::ints(0, instr_time)) {
      auto curcol = ((clock - 1) % 40);

      if (verbose) {
        fmt::print(
            "During Cycle {:>2}: CRT draws pixel in position {} (X = {})\n",
            clock, curcol, X);
      }

      if (curcol == 0) {
        rows.push_back(row);
        row.clear();
      }

      if (curcol == X - 1 || curcol == X || curcol == X + 1) {
        row.push_back('#');
      } else {
        row.push_back('.');
      }

      if (verbose) {
        fmt::print("Current CRT Row: {}\n", row);
      }
      ++clock;

      if (verbose && i != instr_time - 1) {
        fmt::print("\n");
      }
    }

    X += inc;

    if (verbose) {
      std::visit(EndInfoVisitor{clock, X}, instr);
      print_sprite(X);
      fmt::print("\n");
    }
  }

  rows.push_back(row);

  for (auto r : rows) {
    fmt::print("{}\n", r);
  }
}

int main() {
  auto in = input();
  auto instructions = parse(in);

  part1(instructions);
  part2(instructions);

  return 0;
}
