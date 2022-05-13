// This file includes some common algorithms.
#ifndef TICKET_LIB_ALGORITHM_H_
#define TICKET_LIB_ALGORITHM_H_

#include <iostream>
#ifndef ONLINE_JUDGE
#include <algorithm>
#endif // ONLINE_JUDGE

#include "utility.h"

namespace ticket {

using std::distance, std::advance;

#define TICKET_ALGORIGHM_DEFINE_BOUND_FUNC(name, cf) \
template<class Iterator, class T, class Compare = Less<>> \
auto name (Iterator first, Iterator last, const T &value, Compare cmp = {}) \
  -> Iterator { \
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
TICKET_ALGORIGHM_DEFINE_BOUND_FUNC(lowerBound, gt)
#undef TICKET_ALGORIGHM_DEFINE_BOUND_FUNC

template <typename Iterator, class Compare = Less<>>
auto sort (Iterator first, Iterator last, Compare cmp = {})
  -> void {
  // TODO
  std::sort(first, last, [&cmp] (const auto &a, const auto &b) {
    return cmp.lt(a, b);
  });
}

} // namespace ticket

#endif // TICKET_LIB_ALGORITHM_H_
