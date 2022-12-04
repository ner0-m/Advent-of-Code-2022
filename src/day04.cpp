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

bool is_in_range(std::int32_t val, std::int32_t low, std::int32_t high) {
  return low <= val && val <= high;
}

auto unpack(auto x) {
  auto first = std::begin(x);
  auto v1 = *first;
  auto v2 = *ranges::next(first);
  auto v3 = *ranges::next(first, 2);
  auto v4 = *ranges::next(first, 3);

  return std::make_tuple(v1, v2, v3, v4);
}

/// Build list of 4 integers where the first two are for the first range, and
/// the second for the second one
/// i.e from 3-5,4-8 -> [3, 5, 4, 8]
auto process_input(const std::vector<std::string> &in) {
  auto list_of_ranges =
      in | ranges::views::transform([](auto x) {
        auto split = ranges::views::split(x, ',') |
                     ranges::views::transform(
                         [](auto y) { return y | ranges::views::split('-'); }) |
                     ranges::views::transform([](auto y) {
                       return y | ranges::views::transform([](auto z) {
                                return z | ranges::to<std::string>;
                              });
                     });
        auto first = ranges::begin(split);
        auto second = ranges::next(first);

        return std::make_tuple(*first | ranges::to<std::vector>,
                               *second | ranges::to<std::vector>);
      }) |
      ranges::views::transform([](auto t) {
        auto first = std::get<0>(t) | ranges::views::transform(
                                          [](auto x) { return std::stoi(x); });
        auto second = std::get<1>(t) | ranges::views::transform(
                                           [](auto x) { return std::stoi(x); });

        return ranges::views::concat(first, second) | ranges::to<std::vector>;
      });

  return list_of_ranges;
}

/// Idea: compute number of pairs where the first range is in the second, then
/// again when the second range is on the first. Then subtract the number of
/// ranges, which are in both, as they are counted twice
void part1(const std::vector<std::string> &in) {
  auto list_of_ranges = process_input(in);

  auto is_first_in_second = [](auto x) {
    auto [v1, v2, v3, v4] = unpack(x);
    auto first_contained_in_second =
        is_in_range(v1, v3, v4) && is_in_range(v2, v3, v4);
    return first_contained_in_second;
  };

  auto is_second_in_first = [](auto x) {
    auto [v1, v2, v3, v4] = unpack(x);
    auto second_contained_in_first =
        is_in_range(v3, v1, v2) && is_in_range(v4, v1, v2);
    return second_contained_in_first;
  };

  auto is_in_both = [=](auto x) {
    auto [v1, v2, v3, v4] = unpack(x);
    auto second_contained_in_first =
        is_in_range(v3, v1, v2) && is_in_range(v4, v1, v2);
    auto first_contained_in_second =
        is_in_range(v1, v3, v4) && is_in_range(v2, v3, v4);
    return second_contained_in_first && first_contained_in_second;
  };

  // Filter out all elements, expect the ones where the first range is contained
  // in the second
  auto first_in_second =
      list_of_ranges |
      ranges::views::remove_if([=](auto x) { return !is_first_in_second(x); }) |
      ranges::to<std::vector>;

  // Filter out all elements, expect the ones where the second range is
  // contained in the first
  auto second_in_first =
      list_of_ranges |
      ranges::views::remove_if([=](auto x) { return !is_second_in_first(x); }) |
      ranges::to<std::vector>;

  // Filter out all elements, expect the ones, where the ranges are contained in
  // each other (i.e. are equal)
  auto in_both =
      list_of_ranges |
      ranges::views::remove_if([=](auto x) { return !is_in_both(x); }) |
      ranges::to<std::vector>;

  /// Total size is given by the size of each list, minus the ones which are in
  /// both
  auto count = ranges::size(first_in_second) + ranges::size(second_in_first) -
               ranges::size(in_both);
  fmt::print("Number of pairs fully contained in each other: {}\n", count);
}

/// Idea: Compute the pairs with no overlap, then the pairs with overlap is:
/// number of all pairs minus number of pairs with no overlap
void part2(const std::vector<std::string> &in) {
  auto list_of_ranges = process_input(in);

  auto second_overlaps_first = [](auto x) {
    auto [v1, v2, v3, v4] = unpack(x);
    return is_in_range(v3, v1, v2) || is_in_range(v4, v1, v2);
  };

  auto first_overlaps_second = [](auto x) {
    auto [v1, v2, v3, v4] = unpack(x);
    return is_in_range(v1, v3, v4) || is_in_range(v2, v3, v4);
  };

  auto no_overlap =
      list_of_ranges | ranges::views::remove_if([=](auto x) {
        return second_overlaps_first(x) || first_overlaps_second(x);
      }) |
      ranges::to<std::vector>;

  auto size_of_all = ranges::size(list_of_ranges);
  auto count = ranges::size(no_overlap);
  fmt::print("Number of pairs with overlap: {}\n", size_of_all - count);
}

int main() {
  auto in = input();
  part1(in);
  part2(in);
}
