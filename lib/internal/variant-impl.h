// This file contains some undocumented magic on variant.
#ifndef TICKET_LIB_VARIANT_IMPL_H_
#define TICKET_LIB_VARIANT_IMPL_H_

#include <iostream>

#include "utility.h"

namespace ticket::internal {

template <int i>
struct CtorIndex {};
template <int i>
constexpr CtorIndex<i> ctorIndex;

struct CtorValueless {};
inline constexpr CtorValueless ctorValueless;

template <int index, typename ...Types>
union VariantImpl;

template <int index, typename Head, typename ...Tail>
union VariantImpl<index, Head, Tail...> {
  Head head;
  VariantImpl<index + 1, Tail...> tail;

  explicit VariantImpl (CtorValueless) noexcept {} // NOLINT
  ~VariantImpl () {} // NOLINT

  VariantImpl (CtorIndex<0> /* unused */) : head() {}
  template <typename ...Args>
  VariantImpl (CtorIndex<0> /* unused */, Args&& ...args) :
    head(args...) {}

  template <int ix, typename ...Args>
  VariantImpl (CtorIndex<ix> /* unused */, Args&& ...args) :
    tail(ctorIndex<ix - 1>, args...) {}
};

template <int index>
union VariantImpl<index> {};

template <typename Visitor, typename ...Args>
struct VariantVtableImpl;
template <typename Visitor, typename Head, typename ...Tail>
struct VariantVtableImpl<Visitor, Head, Tail...> {
  using FunctionType = void (*)(const Visitor &, void *);
  static auto visitThis (const Visitor &f, void *value) -> void {
    f(*static_cast<Head *>(value));
  }
  static constexpr auto fillVtable (FunctionType *pos) {
    *pos = visitThis;
    VariantVtableImpl<Visitor, Tail...>::fillVtable(pos + 1);
  }
};
template <typename Visitor>
struct VariantVtableImpl<Visitor> {
  template <typename T>
  static constexpr auto fillVtable (T t) {}
};

template <int length, typename T>
struct VariantVtableStore {
  T value[length] = {};
};

template <typename Visitor, typename ...Args>
struct VariantVtable {
  using FunctionType = void (*)(const Visitor &, void *);
  using Store = VariantVtableStore<sizeof...(Args), FunctionType>;
  static constexpr auto generate () -> Store {
    Store store;
    VariantVtableImpl<Visitor, Args...>::fillVtable(store.value);
    return store;
  }
  static constexpr Store vtable = generate();
  static constexpr auto visit (int ix, const Visitor &f, void *value) -> void {
    vtable.value[ix](f, value);
  }
};

template <int i, typename ...Args>
struct NthTypeImpl;
template <typename Head, typename ...Tail>
struct NthTypeImpl<0, Head, Tail...> {
  using type = Head;
};
template <int i, typename Head, typename ...Tail>
struct NthTypeImpl<i, Head, Tail...> {
  using type = typename NthTypeImpl<i - 1, Tail...>::type;
};

template <typename ...Args>
struct VariantTraits;

template <typename T>
struct VariantTraits<T> {
  static constexpr auto hasDuplicates () -> bool {
    return false;
  }
  template <typename U>
  static constexpr auto includes () -> bool {
    return std::is_same_v<T, U>;
  }
  template <typename U>
  static constexpr auto indexOf () -> int {
    if constexpr (std::is_same_v<T, U>) return 0;
    return -1;
  }
  template <int i>
  using NthType = typename NthTypeImpl<i, T>::type;
  template <typename Visitor>
  using Vtable = VariantVtable<Visitor>;
  using Impl = VariantImpl<0>;
};

template <typename Head, typename ...Tail>
struct VariantTraits<Head, Tail...> {
  static constexpr auto hasDuplicates () -> bool {
    return VariantTraits<Tail...>::hasDuplicates() ||
      VariantTraits<Tail...>::template includes<Head>();
  }
  template <typename U>
  static constexpr auto includes () -> bool {
    return std::is_same_v<Head, U> ||
      VariantTraits<Tail...>::template includes<U>();
  }
  template <typename U>
  static constexpr auto indexOf () -> int {
    if constexpr (std::is_same_v<Head, U>) return 0;
    constexpr int i = VariantTraits<Tail...>::template indexOf<U>();
    if constexpr (i < 0) return i;
    return i + 1;
  }
  template <int i>
  using NthType = typename NthTypeImpl<i, Head, Tail...>::type;
  template <typename Visitor>
  using Vtable = VariantVtable<Visitor, Head, Tail...>;
  using Impl = VariantImpl<0, Head, Tail...>;
};

} // namespace ticket::internal

#endif // TICKET_LIB_VARIANT_IMPL_H_
