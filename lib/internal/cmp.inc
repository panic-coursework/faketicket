namespace internal {

struct LessOp {
  template <typename T, typename U>
  auto operator() (const T &lhs, const U &rhs) const -> bool {
    return lhs < rhs;
  }
};

template <typename Lt>
struct GreaterOp {
  template <typename T, typename U>
  auto operator() (const T &lhs, const U &rhs) const -> bool {
    return lt(rhs, lhs);
  }
  Lt lt;
};

} // namespace internal
