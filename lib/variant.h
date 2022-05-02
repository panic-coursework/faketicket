#ifndef TICKET_LIB_VARIANT_H_
#define TICKET_LIB_VARIANT_H_

#include "internal/variant-impl.h"
#include "utility.h"

namespace ticket {

/**
 * @brief A tagged union, aka sum type.
 *
 * This object holds exactly one of its member types, but
 * which type it holds is not statically known. It is
 * entirely on stack, no extra memory allocations are made.
 *
 * Member types need to be unique and not overlapping.
 */
template <typename ...Ts>
class Variant {
 private:
  using Traits = internal::VariantTraits<Ts...>;
  using First = typename Traits::template NthType<0>;
  using Second = typename Traits::template NthType<1>;
  static constexpr size_t length = sizeof...(Ts);
  static_assert(length >= 2);
  static_assert(!Traits::hasDuplicates());
 public:
  Variant () : ix_(0), store_(internal::ctorIndex<0>) {}
  /**
   * constructs the variant from one of its member types.
   */
  template <typename T, int ix = Traits::template indexOf<T>()>
  Variant (const T &value) :
    ix_(ix),
    store_(internal::ctorIndex<ix>, value) {
    static_assert(Traits::template includes<T>());
  }
  Variant (const Variant &other) {
    *this = other;
  }
  Variant (Variant &&other) noexcept { *this = move(other); }
  // this class may be extended, so let it be virtual.
  virtual ~Variant () {
    destroy_();
  }
  auto operator= (const Variant &other) -> Variant & {
    if (this == &other) return *this;
    destroy_();
    ix_ = other.ix_;
    if constexpr (length == 2) {
      if (ix_ == 0) new(&get_<First>()) First(other.get_<First>());
      else new(&get_<Second>()) Second(other.get_<Second>());
    } else {
      other.visit([this] (auto &value) {
        using T = std::remove_cvref_t<decltype(value)>;
        new(&get_<T>()) T(value);
      });
    }
    return *this;
  }
  auto operator= (Variant &&other) noexcept -> Variant & {
    if (this == &other) return *this;
    destroy_();
    ix_ = other.ix_;
    if constexpr (length == 2) {
      if (ix_ == 0) new(&get_<First>()) First(move(other.get_<First>()));
      else new(&get_<Second>()) Second(move(other.get_<Second>()));
    } else {
      other.visit([this] (auto &value) {
        using T = decltype(value);
        new(&get_<T>()) T(move(value));
      });
    }
    return *this;
  }

  /// sets the variant to one of its member types.
  template <typename T, int ix = Traits::template indexOf<T>()>
  auto operator= (const T &value) -> Variant & {
    static_assert(Traits::template includes<T>());
    destroy_();
    ix_ = ix;
    new(&get_<T>()) T(value);
    return *this;
  }

  /// checks if T is the current type of this variant.
  template <typename T>
  auto is () const -> bool {
    static_assert(Traits::template includes<T>());
    return ix_ == Traits::template indexOf<T>();
  }
  /// returns the current index of the current state.
  auto index () const -> int {
    return ix_;
  }

  /// if the current state is of type T, return it. else null.
  template <typename T>
  auto get () -> T * {
    if (is<T>()) return &get_<T>();
    return nullptr;
  }
  /// if the current state is of type T, return it. else null.
  template <typename T>
  auto get () const -> const T * {
    if (is<T>()) return &get_<T>();
    return nullptr;
  }
  /// if the current state is of index ix, return it. else null.
  template <int ix>
  auto get () -> typename Traits::template NthType<ix> * {
    if (ix_ != ix) return nullptr;
    return &get_<typename Traits::template NthType<ix>>();
  }
  /// if the current state is of index ix, return it. else null.
  template <int ix>
  auto get () const -> const typename Traits::template NthType<ix> * {
    if (ix_ != ix) return nullptr;
    return &get_<typename Traits::template NthType<ix>>();
  }

  /**
   * @brief visits the variant using a polymorphic functor.
   *
   * pass in a polymorphic visitor function, and we will
   * call it with the correct type. If the current type is
   * T, then we would call f(T &). Note that this method
   * deliberately disregards const status. This is to ensure
   * that it still works when this is const.
   */
  template <typename Visitor>
  auto visit (const Visitor &f) const -> void {
    using Vt = typename Traits::template Vtable<Visitor>;
    // sorry about the C-style cast here... it casts away const.
    Vt::visit(ix_, f, (void *) &store_);
  }

 private:
  int ix_ = -1;
  typename Traits::Impl store_{internal::ctorValueless};

  template <typename T = void>
  auto get_ () -> T & {
    return *reinterpret_cast<T *>(&store_);
  }
  template <typename T = void>
  auto get_ () const -> const T & {
    return *reinterpret_cast<const T *>(&store_);
  }

  auto destroy_ () -> void {
    if (ix_ == -1) return;
    if constexpr (length == 2) {
      if (ix_ == 0) get_<First>().~First();
      else get_<Second>().~Second();
    } else {
      visit([] (auto &value) {
        using T = std::remove_reference_t<decltype(value)>;
        value.~T();
      });
    }
    ix_ = -1;
  }
};

} // namespace ticket

#endif // TICKET_LIB_VARIANT_H_
