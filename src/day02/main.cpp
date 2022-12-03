#include <iostream>
#include <string>
#include <variant>
#include <vector>

#define FMT_HEADER_ONLY = 1
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <range/v3/all.hpp>

std::vector<std::string> input() {
  std::vector<std::string> in;
  std::string line;
  while (std::getline(std::cin, line)) {
    in.push_back(line);
  }
  return in;
}

struct Rock {};
struct Paper {};
struct Scissors {};

struct Win {};
struct Lose {};
struct Draw {};

using Game = std::variant<Rock, Paper, Scissors>;

bool wins(auto, auto) { return false; }
bool wins(Game x, Game y) {
  return std::visit([](auto lhs, auto rhs) { return wins(lhs, rhs); }, x, y);
}
bool wins(Rock, Scissors) { return true; }
bool wins(Scissors, Paper) { return true; }
bool wins(Paper, Rock) { return true; }

using Outcome = std::variant<Win, Lose, Draw>;

int towin(auto game, auto outcome) {
  return std::visit([](auto lhs, auto rhs) { return towin(lhs, rhs); }, game,
                    outcome);
}

int towin(Rock, Draw) { return 1; }
int towin(Rock, Win) { return 2; }
int towin(Rock, Lose) { return 3; }

int towin(Paper, Draw) { return 2; }
int towin(Paper, Win) { return 3; }
int towin(Paper, Lose) { return 1; }

int towin(Scissors, Draw) { return 3; }
int towin(Scissors, Win) { return 1; }
int towin(Scissors, Lose) { return 2; }

Game to_game(std::string s) {
  if (s == "A") {
    return Rock{};
  } else if (s == "B") {
    return Paper{};
  } else {
    return Scissors{};
  }
}

Outcome to_outcome(std::string s) {
  if (s == "X") {
    return Lose{};
  } else if (s == "Y") {
    return Draw{};
  } else {
    return Win{};
  }
}

int points_for_win(std::string s) {
  if (s == "X") {
    return 0;
  } else if (s == "Y") {
    return 3;
  } else if (s == "Z") {
    return 6;
  } else {
    return 0;
  }
}

int main() {
  auto in = input();

  auto delimited = in | ranges::views::transform([](auto x) {
                     return ranges::views::split(x, ' ') |
                            ranges::to<std::vector<std::string>>;
                   });

  auto results = delimited | ranges::views::transform([](auto x) {
                   return x | ranges::views::drop(1) |
                          ranges::views::transform(points_for_win);
                 }) |
                 ranges::views::join;

  auto won = delimited | ranges::views::transform([](auto x) {
               auto first = ranges::begin(x);
               auto second = ranges::next(first);
               return std::make_tuple(to_game(*first), to_outcome(*second));
             }) |
             ranges::views::transform([](auto t) {
               auto game = std::get<0>(t);
               auto outcome = std::get<1>(t);
               return towin(game, outcome);
             });

  auto res = ranges::accumulate(results, 0) + ranges::accumulate(won, 0);
  fmt::print("You get {} points\n", res);
}
