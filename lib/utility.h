// This file defines several common utilities.
#ifndef TICKET_LIB_UTILITY_H_
#define TICKET_LIB_UTILITY_H_

#include <iostream>

#include "vector.h"

#ifdef TICKET_DEBUG
#include <cassert>
#define TICKET_ASSERT(x) assert(x)
#else
#define TICKET_ASSERT(x)
#endif // TICKET_DEBUG

namespace ticket {

/**
 * @brief splits the string with sep into several substrings.
 *
 * this function mutates the incoming string to make sure
 * the result is properly zero-terminated.
 *
 * the lifetime of the return value is the lifetime of the
 * incoming string; that is to say, you need to keep the
 * original string from destructured in order to use the
 * result.
 */
auto split (std::string &str, char sep)
  -> Vector<std::string_view>;

/// copies the strings in vec into an array of real strings.
auto copyStrings (const Vector<std::string_view> &vec)
  -> Vector<std::string>;

/// An empty class, used at various places.
struct Unit {
  constexpr Unit () = default;
  template <typename T>
  constexpr Unit (const T & /* unused */) {}
  auto operator< (const Unit & /* unused */) -> bool {
    return false;
  }
};
inline constexpr Unit unit;

/// declare value, used in type annotations.
template <typename T>
auto declval () -> T;

/// forcefully make an rvalue.
template <typename T>
auto move (T &val) -> T && {
  return reinterpret_cast<T &&>(val);
}

/// A pair of objects.
template <typename T1, typename T2>
class Pair {
 public:
  T1 first;
  T2 second;
  constexpr Pair () : first(), second() {}
  Pair (const Pair &other) = default;
  Pair (Pair &&other) noexcept = default;
  Pair (const T1 &x, const T2 &y) : first(x), second(y) {}
  template <class U1, class U2>
  Pair (U1 &&x, U2 &&y) : first(x), second(y) {}
  template <class U1, class U2>
  Pair (const Pair<U1, U2> &other) : first(other.first), second(other.second) {}
  template <class U1, class U2>
  Pair (Pair<U1, U2> &&other) : first(other.first), second(other.second) {}
};

/// Comparison utilities.
template <typename Lt>
class Cmp {
 public:
  template <typename T, typename U>
  auto equals (const T &lhs, const U &rhs) -> bool {
    return !lt_(lhs, rhs) && !lt_(rhs, lhs);
  }
  template <typename T, typename U>
  auto ne (const T &lhs, const U &rhs) -> bool {
    return !equals(lhs, rhs);
  }
  template <typename T, typename U>
  auto lt (const T &lhs, const U &rhs) -> bool {
    return lt_(lhs, rhs);
  }
  template <typename T, typename U>
  auto gt (const T &lhs, const U &rhs) -> bool {
    return lt_(rhs, lhs);
  }
  template <typename T, typename U>
  auto leq (const T &lhs, const U &rhs) -> bool {
    return !gt(lhs, rhs);
  }
  template <typename T, typename U>
  auto geq (const T &lhs, const U &rhs) -> bool {
    return !lt(lhs, rhs);
  }
 private:
  Lt lt_;
};

#include "internal/cmp.inc"

template <typename Lt = internal::LessOp>
using Less = Cmp<Lt>;
template <typename Lt = internal::LessOp>
using Greater = Cmp<internal::GreaterOp<Lt>>;

} // namespace ticket

#endif // TICKET_LIB_UTILITY_H_
