/// This file includes some common algorithms.
#ifndef TICKET_LIB_ALGORITHM_H_
#define TICKET_LIB_ALGORITHM_H_

#include <type_traits>

#include "utility.h"

namespace ticket {

template <typename Iterator>
auto distance (Iterator first, Iterator last) -> int {
  int distance = 0;
  while (first != last) {
    ++distance;
    ++first;
  }
  return distance;
}
template <typename Iterator, typename = decltype(declval<Iterator>() - declval<Iterator>())>
auto distance (Iterator first, Iterator last) -> int {
  return last - first;
}

template <typename Iterator>
auto advance (Iterator base, int steps) -> void {
  while (steps-- > 0) ++base;
}
template <typename Iterator, typename = decltype(declval<Iterator>() += declval<int>())>
auto advance (Iterator base, int steps) -> void {
  base += steps;
}

#define TICKET_ALGORIGHM_DEFINE_BOUND_FUNC(name, cf) \
template<class Iterator, class T, class Compare = Less<T>> \
auto name (Iterator first, Iterator last, const T &value, Compare cmp = {}) -> Iterator { \
  int length = distance(first, last); \
  while (length != 0) { \
    auto it = first; \
    int mid = length / 2; \
    advance(it, mid); \
    if (cmp.cf(value, *it)) { \
      first = ++it; \
      length -= mid + 1; \
    } else { \
      length = mid; \
    } \
  } \
  return first; \
}
TICKET_ALGORIGHM_DEFINE_BOUND_FUNC(upperBound, geq)
TICKET_ALGORIGHM_DEFINE_BOUND_FUNC(lowerBound, ge)
#undef TICKET_ALGORIGHM_DEFINE_BOUND_FUNC

} // namespace ticket

#endif // TICKET_LIB_ALGORITHM_H_
