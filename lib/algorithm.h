// This file includes some common algorithms.
#ifndef TICKET_LIB_ALGORITHM_H_
#define TICKET_LIB_ALGORITHM_H_

#include <iostream>

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

/// sorts the elements between first and last.
template <typename Iterator, class Compare = Less<>>
auto sort (Iterator first, Iterator last, Compare cmp = {})
  -> void {
  auto distance = std::distance(first, last);
  if (distance <= 1) return;
  auto mid = first;
  std::advance(mid, distance / 2);
  sort(first, mid, cmp);
  sort(mid, last, cmp);
  std::remove_cvref_t<decltype(*first)> tmp[distance + 1];
  int s = 0;
  auto p = first;
  auto q = mid;
  while (s < distance) {
    if (p != mid && (q == last || cmp.lt(*p, *q))) {
      tmp[s++] = *p++;
    } else {
      tmp[s++] = *q++;
    }
  }
  int i = 0;
  while (first != last) *first++ = tmp[i++];
  TICKET_ASSERT(i == distance);
}

} // namespace ticket

#endif // TICKET_LIB_ALGORITHM_H_
