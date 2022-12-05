#include <deque>
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

std::vector<std::deque<char>>
process_start_stack(const std::vector<std::string> &in) {
  std::vector<std::string> stacks(in.begin(), in.begin() + 8);

  auto cleaned_stacks = stacks | ranges::views::transform([](auto x) {
                          // every 4 elements there is a new part of the stack
                          return x | ranges::views::chunk(4) |
                                 ranges::views::transform([](auto y) {
                                   // Remove all brackets and whitespace
                                   return y | ranges::views::filter([](auto c) {
                                            return std::isalpha(c);
                                          }) |
                                          ranges::to<std::vector>;
                                 }) |
                                 // Replace empty ranges with a placeholder,
                                 // else just return the single element left
                                 ranges::views::transform([](auto x) {
                                   if (ranges::empty(x)) {
                                     return '-';
                                   }
                                   return *ranges::begin(x);
                                 }) |
                                 ranges::to<std::vector>;
                        });

  // Basically transpose the stacks and put them in a stack
  std::vector<std::deque<char>> s(9);
  ranges::for_each(cleaned_stacks | ranges::views::reverse, [&](auto x) {
    ranges::for_each(x | ranges::views::enumerate, [&](auto t) {
      auto i = std::get<0>(t);
      auto c = std::get<1>(t);
      if (c != '-') {
        s[i].push_front(c);
      }
    });
  });

  return s;
}

/// Process list of commands each of the form "move x from y to z" -> [x, y, z]
std::vector<std::vector<int>>
process_commands(const std::vector<std::string> &in) {
  std::vector<std::string> commands(in.begin() + 10, in.end());

  auto cmd =
      commands | ranges::views::transform([](auto x) {
        // Split by spaces, then ensure that words are stored as string (avoid
        // dangling), then remove all words, which are not numbers, then convert
        // the strings to numbers
        return x | ranges::views::split(' ') |
               ranges::views::transform(
                   [](auto word) { return word | ranges::to<std::string>; }) |
               ranges::views::remove_if([](auto s) {
                 return !ranges::accumulate(s, true, [](auto accu, auto c) {
                   return accu && std::isdigit(static_cast<unsigned char>(c));
                 });
               }) |
               ranges::views::transform(
                   [](auto num) -> int { return std::stoi(num); }) |
               ranges::to<std::vector>;
      }) |
      ranges::to<std::vector>;

  return cmd;
}

auto execute_commands(std::vector<std::deque<char>> stacks,
                      const std::vector<std::vector<int>> &cmds, auto move_fn) {
  auto unpack = [](auto t) {
    auto first = ranges::begin(t);
    auto second = ranges::next(first);
    auto third = ranges::next(second);
    return std::make_tuple(*first, *second, *third);
  };

  // Helper to pretty print the current stack
  auto print_stack = [](auto stack) {
    ranges::for_each(stack | ranges::views::enumerate, [](auto t) {
      fmt::print("{}: {}\n", std::get<0>(t) + 1,
                 std::get<1>(t) | ranges::views::reverse);
    });
  };

  // Helper to print the command
  auto print_cmd = [&](auto i, auto cmd) {
    auto [num, from, to] = unpack(cmd);
    fmt::print("{}: move {} from {} to {}\n", i, num, from, to);
  };

  // For each command, call the move function
  ranges::for_each(cmds, [&](auto cmd) {
    auto [num, from, to] = unpack(cmd);
    move_fn(stacks, num, from, to);
  });

  /* If I want to debug I can use this:
  auto tmp = cmds | ranges::views::enumerate;
  ranges::for_each(tmp, [&](auto t) {
    auto i = std::get<0>(t);
    auto cmd = std::get<1>(t);
    auto [num, from, to] = unpack(cmd);

    print_cmd(i, cmd);
    print_stack(stacks);
    move_fn(stacks, num, from, to);
    fmt::print("\n");
    print_stack(stacks);
    fmt::print("\n");
  });
  */

  /// remove all but the first element of the stack (i.e. the top most element),
  /// convert this to a string and return it
  return stacks | ranges::views::transform([](auto x) {
           auto t = x | ranges::views::take(1);
           return static_cast<char>(*ranges::begin(t));
         }) |
         ranges::to<std::string>;
}

auto split_stacks(const std::vector<std::string> &in) {
  auto stacks = process_start_stack(in);
  auto commands = process_commands(in);

  return std::make_tuple(stacks, commands);
}

void part1(const std::vector<std::string> &in) {
  auto [stacks, commands] = split_stacks(in);

  // In part 1, each element is moved element by element
  auto move = [](auto &stacks, auto num, auto from, auto to) {
    // copy the first num elements from the source stack to the destination
    // stack
    ranges::copy(stacks[from - 1] | ranges::views::take(num),
                 std::front_inserter(stacks[to - 1]));

    // now delete the elements from the source stack
    stacks[from - 1].erase(ranges::begin(stacks[from - 1]),
                           ranges::begin(stacks[from - 1]) + num);
  };
  auto top = execute_commands(stacks, commands, move);

  fmt::print("Top of final stack: {}\n", top);
}

void part2(const std::vector<std::string> &in) {
  auto [stacks, commands] = split_stacks(in);

  auto move = [](auto &stacks, auto num, auto from, auto to) {
    // copy the first num elements in reversed order from the source stack to
    // the destination stack
    ranges::copy(stacks[from - 1] | ranges::views::take(num) |
                     ranges::views::reverse,
                 std::front_inserter(stacks[to - 1]));

    // now delete the elements from the source stack
    stacks[from - 1].erase(ranges::begin(stacks[from - 1]),
                           ranges::begin(stacks[from - 1]) + num);
  };

  auto top = execute_commands(stacks, commands, move);

  fmt::print("Top of final stack with CrateMover 9001: {}\n", top);
}

int main() {
  auto in = input();
  part1(in);
  part2(in);
}
