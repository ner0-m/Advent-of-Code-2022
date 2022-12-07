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

struct File {
  std::size_t size;
  std::string name;
};

struct Dir {
  std::string name;
  std::size_t size;
  std::vector<Dir> dirs{};
  std::vector<File> files{};
  Dir *toplevel = nullptr;
};

struct CommandCD {
  Dir argument;
};

struct CommandCDUp {};

using Command = std::variant<CommandCD, CommandCDUp>;
using Commands = std::vector<Command>;
using Input = std::variant<File, Dir, Command>;

auto parse_input(const std::vector<std::string> &in) {
  using namespace std::string_view_literals;

  std::vector<Input> input;

  for (auto line : in) {
    if (line.front() == '$') {
      if (line.starts_with("$ cd"sv)) {
        auto space_pos = line.rfind(' ');

        const auto argument = line.substr(space_pos + 1, line.size());
        if (argument == ".."sv) {
          input.push_back(CommandCDUp{});
        } else {
          input.push_back(CommandCD{argument, 0});
        }
      }
    } else if (line.starts_with("dir"sv)) {
      auto space_pos = line.find(' ');
      input.push_back(Dir{line.substr(space_pos + 1, line.size()), 0});
    } else {
      auto space_pos = line.find(' ');
      auto size = std::stoul(line.substr(0, space_pos));
      auto name = line.substr(space_pos, line.size());
      input.push_back(File{size, name});
    }
  }
  return input;
}

struct CommandVisitor {
  auto operator()(const CommandCD &cmd) const {
    auto &dirs = curdir->dirs;
    auto it = std::find_if(dirs.begin(), dirs.end(), [&](auto dir) {
      return dir.name == cmd.argument.name;
    });

    // I know I should check for end iterator, but I know it will find something
    // here due the assignment, but I'm not checking for nullptr anywhere
    // anyway, so I DON'T CARE :D
    return &(*it);
  }

  auto operator()(CommandCDUp) const { return curdir->toplevel; }

  Dir *curdir;
};

struct InputVisitor {
public:
  auto operator()(const Command &cmd) {
    curdir = std::visit(CommandVisitor{curdir}, cmd);
    return curdir;
  }

  auto operator()(const Dir &dir) {
    curdir->dirs.push_back(dir);
    curdir->dirs.back().toplevel = curdir;
    return curdir;
  };
  auto operator()(const File &file) {
    curdir->files.push_back(file);
    return curdir;
  };

  Dir *curdir;
};

// This is damn ugly, I don't like it p.q
std::size_t dirsize(Dir &dir) {
  auto size = 0;
  for (auto f : dir.files) {
    size += f.size;
  }
  for (auto &d : dir.dirs) {
    auto s = dirsize(d);
    d.size = s;
    size += s;
  }
  dir.size = size;
  return size;
}

Dir populate_filesystem(const std::vector<Input> &input) {
  Dir root{"/", 0, {}, {}, nullptr};
  Dir *curdir = &root;
  for (auto inp : input | ranges::views::drop(1)) {
    curdir = std::visit(InputVisitor{curdir}, inp);
  }

  // determine size of each folder
  dirsize(root);

  return root;
}

auto reducedir(const Dir &dir, auto pred, auto fn, auto init)
    -> decltype(init) {
  auto accum = init;

  if (pred(dir)) {
    accum = fn(accum, dir);
  }

  for (auto &d : dir.dirs) {
    accum = reducedir(d, pred, fn, accum);
  }

  return accum;
}

// Nice, I like this
std::size_t sumsmall(const Dir &dir) {
  return reducedir(
      dir, [](auto d) { return d.size <= 100'000; },
      [](auto accum, auto dir) { return accum + dir.size; }, 0);
}

// This is damn ugly, I don't like it p.q
std::size_t smallest_dir(const Dir &dir, std::size_t current_smallest,
                         std::size_t remove_at_least) {
  for (const auto &d : dir.dirs) {
    current_smallest = smallest_dir(d, current_smallest, remove_at_least);
  }

  if (dir.size >= remove_at_least && dir.size < current_smallest) {
    current_smallest = dir.size;
  }
  return current_smallest;
}

void part1(const Dir &root) {
  fmt::print("The sum of dirs smaller than 100'000: {}\n", sumsmall(root));
}

void part2(const Dir &root) {
  constexpr auto total_space = 70'000'000;
  constexpr auto necessary_space = 30'000'000;

  auto used_space = root.size;
  auto free_space = total_space - used_space;
  auto remove_at_least = necessary_space - free_space;

  auto free = smallest_dir(root, root.size, remove_at_least);
  fmt::print("You need to delete a folder of size: {}\n", free);
}

int main() {
  auto in = input();
  auto input = parse_input(in);
  auto root = populate_filesystem(input);

  part1(root);
  part2(root);
}
