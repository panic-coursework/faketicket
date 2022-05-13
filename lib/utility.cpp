#include "utility.h"

namespace ticket {

auto split (std::string &str, char sep)
  -> Vector<std::string_view> {
  Vector<std::string_view> res;
  auto cstr = str.c_str();
  int start = 0;
  for (int i = 0; i < str.length(); ++i) {
    if (str[i] == ' ') {
      str[i] = '\0';
      if (i != start) {
        res.push_back(std::string_view(cstr + start));
      }
      start = i + 1;
    }
  }
  if (start != str.length()) {
    res.push_back(std::string_view(cstr + start));
  }
  return res;
}

auto copyStrings (const Vector<std::string_view> &vec)
  -> Vector<std::string> {
  return vec.map([] (const auto &x) {
    return std::string(x);
  });
}

}
