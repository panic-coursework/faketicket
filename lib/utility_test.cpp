#include "utility.h"

#include <cassert>
#include <vector>

using ticket::split, ticket::copyStrings;

auto main () -> int {
  std::string str = "1926 0817  hello world !!!!!";
  auto vec = split(str, ' ');
  std::vector<std::string> std = { "1926", "0817", "hello", "world", "!!!!!" };
  assert(vec.size() == 5);
  for (int i = 0; i < std.size(); ++i) {
    assert(vec[i] == std[i]);
    assert(*vec[i].end() == '\0');
  }
  auto copy = copyStrings(vec);
  assert(copy.size() == 5);
  for (int i = 0; i < std.size(); ++i) {
    assert(copy[i] == std[i]);
    assert(copy[i] == vec[i]);
  }
  return 0;
}
