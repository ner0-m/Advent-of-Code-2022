#include <charconv>
#include <functional>
#include <iostream>
#include <optional>
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

struct Plus {};
struct Multiplies {};
using OpKind = std::variant<Plus, Multiplies>;

using uint128_t = unsigned __int128;

struct Monkey {
  std::vector<uint128_t> items;

  std::function<uint128_t(uint128_t)> operation;
  std::optional<uint128_t> oparg;
  OpKind opKind;

  std::function<bool(uint128_t)> test;
  uint128_t testarg;

  int throw_to_if_true;
  int throw_to_if_false;

  std::vector<uint128_t> inspected_items_per_round{};
  uint128_t inspected_items_count = 0;
};

std::vector<Monkey> parse_input(const std::vector<std::string> &in) {
  std::vector<std::vector<std::string>> monkey_text;
  auto first = in.begin();
  auto last = in.end();

  while (first != last) {
    auto it = std::find(first, in.end(), "");

    auto copy = std::vector<std::string>(first, it);
    monkey_text.push_back(copy);

    if (it == last) {
      break;
    }

    first = it + 1;
  }

  auto parse_items = [](auto x) {
    auto it = std::find(x.begin(), x.end(), ':');

    auto range = ranges::make_subrange(it + 1, x.end());
    auto ints = range | ranges::views::split(',') |
                ranges::views::transform([](auto x) {
                  auto tmp = x | ranges::views::remove_if([](auto c) {
                               return std::isspace(c);
                             }) |
                             ranges::to<std::string>;
                  return uint128_t(std::stoi(tmp));
                }) |
                ranges::to<std::vector>;
    return ints;
  };

  auto parse_operation =
      [](auto x) -> std::tuple<std::function<uint128_t(uint128_t)>,
                               std::optional<int>, OpKind> {
    auto it = ranges::find(x, '=');

    auto range = ranges::make_subrange(it + 2, x.end());

    // This is always 'old'
    auto first_arg =
        ranges::make_subrange(ranges::begin(range), ranges::begin(range) + 3);
    auto plus = ranges::find(range, '+');
    if (plus != ranges::end(x)) {
      auto second_arg = ranges::make_subrange(plus + 2, ranges::end(range));
      if (ranges::equal(second_arg, "old")) {
        return {[](auto x) { return x + x; }, std::nullopt, Plus{}};
      }

      auto arg = std::stoi(second_arg | ranges::to<std::string>);
      return {[arg](auto x) { return x + arg; }, arg, Plus{}};
    }

    auto multiplies = ranges::find(range, '*');
    if (multiplies != ranges::end(x)) {
      auto second_arg =
          ranges::make_subrange(multiplies + 2, ranges::end(range)) |
          ranges::to<std::string>;
      if (second_arg == "old") {
        return {[](auto x) { return x * x; }, std::nullopt, Multiplies{}};
      }

      auto arg = std::stoi(second_arg | ranges::to<std::string>);
      return {[arg](auto x) { return x * arg; }, arg, Multiplies{}};
    }

    fmt::print("Could not parse: {}\n", range);
    return {[](auto) { return -666666; }, std::nullopt, Plus{}};
  };

  auto parse_last_int = [](auto x) -> int {
    auto last_space = x.rfind(' ') + 1;
    auto subrange =
        ranges::make_subrange(x.begin() + last_space, ranges::end(x)) |
        ranges::to<std::string>;

    return std::stoi(subrange);
  };

  auto parse_test =
      [=](auto x) -> std::tuple<std::function<bool(uint128_t)>, uint128_t> {
    auto arg = parse_last_int(x);
    return {[arg](auto x) { return x % arg == 0; }, arg};
  };

  std::vector<Monkey> monkeys;
  for (auto monkey : monkey_text) {
    fmt::print("{}\n", monkey);
    auto ints = parse_items(monkey[1]);
    auto [op, arg, kind] = parse_operation(monkey[2]);
    // std::function<int(int)> operation = parse_operation(monkey[2]);
    // std::function<bool(int)> test = parse_test(monkey[3]);
    auto [test, testarg] = parse_test(monkey[3]);
    int if_true = parse_last_int(monkey[4]);
    int if_false = parse_last_int(monkey[5]);

    monkeys.push_back(
        Monkey{ints, op, arg, kind, test, testarg, if_true, if_false});
  }
  return monkeys;
}

void simulate_round(std::vector<Monkey> &monkeys,
                    std::optional<uint128_t> supermodulo,
                    bool do_division = false, bool verbose = false) {
  for (auto [i, monkey] : ranges::views::enumerate(monkeys)) {
    monkey.inspected_items_per_round.resize(
        monkey.inspected_items_per_round.size() + 1);

    if (verbose) {
      fmt::print("Monkey {}:\n", i);
    }

    for (auto item : monkey.items) {
      if (verbose) {
        fmt::print("  Monkey inspects an item with a worry level of {}\n",
                   item);
      }

      uint128_t newlevel = [&](auto monkey) {
        if (supermodulo.has_value()) {
          return monkey.operation(item) % *supermodulo;
        } else {
          return monkey.operation(item);
        }
      }(monkey);

      if (verbose) {
        auto oparg = [item](auto x) {
          if (x.has_value()) {
            return *x;
          } else {
            return item;
          }
        }(monkey.oparg);

        std::string opstring =
            std::visit(overloaded{[](Plus) { return "increases"; },
                                  [](Multiplies) { return "is multiplied"; }},
                       monkey.opKind);
        fmt::print("    Worry level {} by {} to {}.\n", opstring, oparg,
                   newlevel);
      }

      if (do_division) {
        newlevel /= 3;
        if (verbose) {
          fmt::print(
              "    Monkey gets bored with item. Worry level is divided by 3 "
              "to {}.\n",
              newlevel);
        }
      }

      auto throw_to = [&](auto monkey) {
        if (monkey.test(newlevel)) {
          if (verbose) {
            fmt::print("    Current worry level is divisible by {}.\n",
                       monkey.testarg);

            fmt::print("    Item with worry level {} is thrown to monkey {}.\n",
                       newlevel, monkey.throw_to_if_true);
          }
          return monkey.throw_to_if_true;
        } else {
          if (verbose) {
            fmt::print("    Current worry level is not divisible by {}.\n",
                       monkey.testarg);
            fmt::print("    Item with worry level {} is thrown to monkey {}.\n",
                       newlevel, monkey.throw_to_if_false);
          }
          return monkey.throw_to_if_false;
        }
      }(monkey);

      ++monkey.inspected_items_count;
      monkeys[throw_to].items.push_back(newlevel);
    }
    monkey.items.clear();
  }
}

std::pair<uint128_t, uint128_t> top2(const std::vector<Monkey> &monkeys) {
  // TODO: This could be nicer
  auto top2 = std::pair<uint128_t, uint128_t>{0, 0};
  for (auto [i, monkey] : monkeys | ranges::views::enumerate) {
    auto count = monkey.inspected_items_count;
    fmt::print("Monkey {} inspected items {} times\n", i, count);

    if (count > top2.first) {
      std::swap(top2.first, top2.second);
      top2.first = count;
    } else if (count > top2.second) {
      top2.second = count;
    }
  }
  return top2;
}

void part1(std::vector<Monkey> monkeys) {
  for (auto round : ranges::views::iota(1, 21)) {
    // fmt::print("Round {}\n", round);
    simulate_round(monkeys, {}, true);

    // fmt::print("After round {}, the monkeys are holding items with these
    // worry "
    //            "levels:\n",
    //            round);
    // for (auto [i, monkey] : monkeys | ranges::views::enumerate) {
    //   fmt::print("Monkey {}: {}\n", i, monkey.items);
    // }
  }

  auto top = top2(monkeys);
  fmt::print("monkey business: {}\n\n\n", top.first * top.second);
}

void part2(std::vector<Monkey> monkeys) {
  auto supermodulo =
      ranges::accumulate(monkeys, 1, std::multiplies{}, &Monkey::testarg);

  for (auto round : ranges::views::iota(1, 10001)) {
    if (round % 1000 == 0) {
      fmt::print("Round {}\n", round);
    }
    simulate_round(monkeys, supermodulo);

    if (round == 20 || round % 1000 == 0) {
      fmt::print(
          "After round {}, the monkeys are holding items with these worry "
          "levels:\n",
          round);
      for (auto [i, monkey] : monkeys | ranges::views::enumerate) {
        fmt::print("Monkey {}: {}\n", i, monkey.items);
      }
    }
  }

  auto top = top2(monkeys);
  fmt::print("monkey business: {}\n", top.first * top.second);
}

int main() {
  auto in = input();
  auto monkeys = parse_input(in);

  part1(monkeys);
  part2(monkeys);

  return 0;
}
