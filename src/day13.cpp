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
#include <fmt/std.h>
#include <range/v3/all.hpp>

#include "input.hpp"
#include "overloaded.hpp"

struct List {

  List() = default;
  List(int i) : list_(i) {}
  List(std::vector<List> l) : list_(l) {}

  std::vector<List> &list() { return std::get<std::vector<List>>(list_); }
  const std::vector<List> &list() const {
    return std::get<std::vector<List>>(list_);
  }

  int Int() const { return std::get<int>(list_); }

  std::variant<std::vector<List>, int> list_{};
  List *parent = nullptr;
};

template <> struct fmt::formatter<List> {
  constexpr auto parse(format_parse_context &ctx) const { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const List &list, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    auto out = ctx.out();
    if (std::holds_alternative<std::vector<List>>(list.list_)) {
      return fmt::format_to(out, "{}", list.list());
    } else {
      return fmt::format_to(out, "{}", list.Int());
    }
  }
};

using Packet = std::pair<List, List>;

// This is the only, pretty ugly way to parse the recursive form
auto parse_list(auto in, auto &list) -> decltype(ranges::begin(in)) {
  auto &l = list.list();

  auto first = ranges::begin(in);

  if (*first == '[') {
    // We found a new list, parse it
    List newlist = std::vector<List>{};
    newlist.parent = &list;
    l.emplace_back(newlist);

    auto rng = ranges::make_subrange(ranges::next(first), ranges::end(in));
    return parse_list(rng, l.back());
  } else if (*first == ']') {
    // If not the outermost list, remove the next comma
    if (list.parent) {
      auto next = ranges::next(first);
      if (*next == ',') {
        first = ranges::next(next);
      } else {
        first = next;
      }

      return parse_list(ranges::make_subrange(first, ranges::end(in)),
                        *list.parent);
    }
  } else {
    // Parse numbers
    auto rng = ranges::make_subrange(first, ranges::end(in));
    auto end_of_number =
        ranges::find_if_not(rng, [](auto x) { return std::isalnum(x); });

    auto numrng = ranges::make_subrange(first, end_of_number);
    auto num = numrng | ranges::to<std::string>;

    // This somehow works, not sure why :D
    try {
      l.push_back(std::stoi(num));
    } catch (...) {
      return end_of_number;
    }

    if (*end_of_number == ',') {
      first = ranges::next(end_of_number);
    } else {
      first = end_of_number;
    }

    return parse_list(ranges::make_subrange(first, ranges::end(in)), list);
  }

  return first;
}

List parse(auto in) {
  List lists = std::vector<List>{};

  auto first = ranges::begin(in);
  while (first != ranges::end(in)) {
    List l = std::vector<List>{};

    if (*first == '[') {
      auto rng = ranges::make_subrange(ranges::next(first), ranges::end(in));
      first = parse_list(rng, l);
    } else {
      ++first;
    }

    if (!l.list().empty()) {
      lists.list().emplace_back(l);
    }
  }
  if (!lists.list().empty()) {
    return lists.list().front();
  } else {
    return lists.list();
  }
}

std::vector<List> parse_input(const std::vector<std::string> &in) {
  auto list = std::vector<List>{};

  for (auto x : in | ranges::views::split("")) {
    auto first = ranges::begin(x);
    auto second = ranges::next(first);

    list.emplace_back(parse(*first));
    list.emplace_back(parse(*second));
  }
  return list;
}

std::weak_ordering compare_order(List lhs, List rhs, int indent = 0,
                                 bool verbose = false) {
  if (verbose) {
    fmt::print("{:{}}- Compare {} vs {}\n", " ", indent, lhs, rhs);
  }

  // We got two int, use <=>, nice!
  if (std::holds_alternative<int>(lhs.list_) &&
      std::holds_alternative<int>(rhs.list_)) {
    auto res = lhs.Int() <=> rhs.Int();
    if (verbose) {
      if (res == std::weak_ordering::less) {
        fmt::print(
            "{:{}}- Left side is smaller, so inputs are in the right order\n",
            " ", indent + 2);
      } else if (res == std::weak_ordering::greater) {
        fmt::print("{:{}}- Right side is smaller, so inputs are *not* in the "
                   "right order\n",
                   " ", indent + 2);
      }
    }
    return res;
  }

  // Unpack if either lhs or rhs is a list
  std::vector<List> lhs_list;
  if (std::holds_alternative<int>(lhs.list_)) {
    lhs_list.push_back(lhs.Int());
  } else {
    lhs_list = lhs.list();
  }

  std::vector<List> rhs_list;
  if (std::holds_alternative<int>(rhs.list_)) {
    rhs_list.push_back(rhs.Int());
  } else {
    rhs_list = rhs.list();
  }

  // Check the list for the same lengths ones. If any are not equal, we can
  // already stop
  for (auto [l, r] : ranges::views::zip(lhs_list, rhs_list)) {
    auto res = compare_order(l, r, indent + 2, verbose);
    if (res != std::weak_ordering::equivalent) {
      return res;
    }
  }

  // If lhs is smaller, ordering is fine
  if (lhs_list.size() < rhs_list.size()) {
    if (verbose) {
      fmt::print(
          "{:{}}- Left side is smaller, so inputs are in the right order.\n",
          " ", indent + 2);
    }
    return std::weak_ordering::less;
  }

  // if rhs is smaller, order is not fine
  if (lhs_list.size() > rhs_list.size()) {
    if (verbose) {
      fmt::print(
          "{:{}}- Right side is smaller, so inputs are *not* in the right "
          "order\n",
          " ", indent + 2);
    }
    return std::weak_ordering::greater;
  }

  // else they are equaivalent
  return std::weak_ordering::equivalent;
}

void part1(const std::vector<std::string> &in) {
  auto list = parse_input(in);

  auto sum = 0;
  for (auto [i, p] :
       list | ranges::views::chunk(2) | ranges::views::enumerate) {
    auto p1 = *ranges::begin(p);
    auto p2 = *ranges::next(ranges::begin(p));
    fmt::print("=== Pair {} ===\n", i + 1);

    if (compare_order(p1, p2, 0, true) == std::weak_ordering::less) {
      sum += i + 1;
    }
  }
  fmt::print("Sum of correctly ordered packats: {}\n", sum);
}

void part2(const std::vector<std::string> &in) {
  auto list = parse_input(in);

  List divider1 = std::vector<List>{std::vector<List>{2}};
  List divider2 = std::vector<List>{std::vector<List>{6}};
  list.push_back(divider1);
  list.push_back(divider2);

  ranges::sort(list, [](List a, List b) {
    return compare_order(a, b, 0) == std::weak_ordering::less;
  });

  int decoderKey = 1;
  for (auto [i, l] : list | ranges::views::enumerate) {
    if (compare_order(l, divider1) == std::weak_ordering::equivalent ||
        compare_order(l, divider2) == std::weak_ordering::equivalent)
      decoderKey *= (i + 1);
  }
  fmt::print("Decoder key: {}\n", decoderKey);
}

int main() {
  auto in = input();

  part1(in);
  part2(in);

  return 0;
}
