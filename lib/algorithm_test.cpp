#include "algorithm.h"

#include <iostream>

#include "vector.h"

auto main () -> int {
  int cases;
  std::ios::sync_with_stdio(false);
  std::cin.tie(NULL);
  std::cin >> cases;
  for (int i = 0; i < cases; ++i) {
    ticket::Vector<int> vec;
    int length;
    std::cin >> length;
    vec.reserve(length);
    for (int j = 0; j < length; ++j) {
      int x;
      std::cin >> x;
      vec.push_back(x);
    }
    ticket::sort(vec.begin(), vec.end());
    std::cout << length << '\n';
    for (int x : vec) std::cout << x << ' ';
    std::cout << '\n';
  }
  return 0;
}
