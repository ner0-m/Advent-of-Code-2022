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

int priority(char c) {
  if (c >= 'a' && c <= 'z') {
    // [a-z]
    return static_cast<int>(c) - 97 + 1;
  }
  // [A-Z]
  return static_cast<int>(c) - 65 + 27;
}

std::string contains(std::string_view s1, std::string_view s2) {
  std::string contained;
  for (auto c : s1) {
    auto pos = std::find(s2.begin(), s2.end(), c);

    if (pos != s2.end()) {
      contained.push_back(c);
    }
  }
  return contained;
}

void sum_of_priorities(const std::vector<std::string> &in) {
  auto halfes =
      in | ranges::views::transform([](auto x) {
        auto size = ranges::size(x) / 2;
        return std::make_tuple(x.substr(0, size), x.substr(size, x.size()));
      }) |
      ranges::views::transform([](auto t) {
        auto first = std::move(std::get<0>(t)) | ranges::action::sort |
                     ranges::action::unique;
        auto second = std::move(std::get<1>(t)) | ranges::action::sort |
                      ranges::action::unique;
        return std::make_tuple(first, second);
      }) |
      ranges::views::transform([](auto t) {
        auto first = std::get<0>(t);
        auto second = std::get<1>(t);
        return contains(first, second);
      }) |
      ranges::views::transform([](auto s) {
        return ranges::accumulate(
            s | ranges::views::transform([](auto c) { return priority(c); }),
            0);
      });

  auto sum = ranges::accumulate(halfes, 0);

  fmt::print("Sum of priorities: {}\n", sum);
}

void sum_of_badge_priorities(const std::vector<std::string> &in) {

  auto halfes =
      in | ranges::views::transform([](auto x) {
        return std::move(x) | ranges::action::sort | ranges::action::unique;
      }) |
      ranges::views::chunk(3) | ranges::views::transform([](auto x) {
        auto first = ranges::begin(x);
        return fmt::format("{}{}{}", *first, *ranges::next(first),
                           *ranges::next(first, 2));
      }) |
      ranges::views::transform(
          [](auto x) { return std::move(x) | ranges::action::sort; }) |
      ranges::to<std::vector>;

  auto chunked = halfes | ranges::views::transform([](auto x) {
                   auto slided =
                       x | ranges::views::sliding(3) |
                       ranges::views::transform(
                           [](auto y) { return y | ranges::views::unique; }) |
                       ranges::views::remove_if([](auto y) {
                         return ranges::size(y | ranges::to<std::vector>) != 1;
                       }) |
                       ranges::views::transform(
                           [](auto y) { return priority(*ranges::begin(y)); });
                   return slided | ranges::to<std::vector>;
                 }) |
                 ranges::views::join | ranges::to<std::vector>;

  fmt::print("Sum of badge priority: {}\n", chunked | ranges::views::take(10));
  auto sum = ranges::accumulate(chunked, 0);
  fmt::print("Sum of badge priority: {}\n", sum);
}

int main() {
  auto in = input();
  sum_of_priorities(in);
  sum_of_badge_priorities(in);
}
