namespace internal {

// Resembles __map_value_compare in libc++.
template <typename Key, typename Value, typename Cmp>
class MapValueCompare {
 private:
  using Pair = ticket::Pair<const Key, Value>;
  Cmp cmp_;
 public:
  auto operator() (const Key &lhs, const Key &rhs) const -> bool {
    return cmp_(lhs, rhs);
  }
  auto operator() (const Key &lhs, const Pair &rhs) const -> bool {
    return cmp_(lhs, rhs.first);
  }
  auto operator() (const Pair &lhs, const Key &rhs) const -> bool {
    return cmp_(lhs.first, rhs);
  }
  auto operator() (const Pair &lhs, const Pair &rhs) const -> bool {
    return cmp_(lhs.first, rhs.first);
  }
};

} // namespace internal
