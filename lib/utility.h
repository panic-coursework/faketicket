/// This file defines several common utilities.
#ifndef TICKET_LIB_UTILITY_H_
#define TICKET_LIB_UTILITY_H_

#ifdef TICKET_DEBUG
#include <cassert>
#define TICKET_ASSERT(x) assert(x)
#else
#define TICKET_ASSERT(x)
#endif // TICKET_DEBUG

namespace ticket {

/// An empty class, used at various places.
struct Unit {
  constexpr Unit () = default;
  template <typename T>
  constexpr Unit (const T & /* unused */) {}
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
template <class T1, class T2>
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
template <typename T, typename Lt>
class Cmp {
 public:
  auto equals (const T &lhs, const T &rhs) -> bool {
    return !lt_(lhs, rhs) && !lt_(rhs, lhs);
  }
  auto lt (const T &lhs, const T &rhs) -> bool {
    return lt_(lhs, rhs);
  }
  auto gt (const T &lhs, const T &rhs) -> bool {
    return lt_(rhs, lhs);
  }
  auto leq (const T &lhs, const T &rhs) -> bool {
    return !gt(lhs, rhs);
  }
  auto geq (const T &lhs, const T &rhs) -> bool {
    return !lt(lhs, rhs);
  }
 private:
  Lt lt_;
};

namespace internal {

template <typename T>
struct LessOp {
  auto operator() (const T &lhs, const T &rhs) const -> bool {
    return lhs < rhs;
  }
};

template <typename T, typename Lt>
struct GreaterOp {
  auto operator() (const T &lhs, const T &rhs) const -> bool {
    return lt(rhs, lhs);
  }
  Lt lt;
};

} // namespace internal

template <typename T, typename Lt = internal::LessOp<T>>
using Less = Cmp<T, Lt>;
template <typename T, typename Lt = internal::LessOp<T>>
using Greater = Cmp<T, internal::GreaterOp<T, Lt>>;

} // namespace ticket

#endif // TICKET_LIB_UTILITY_H_
