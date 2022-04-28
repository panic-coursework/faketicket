#include "variant.h"

#include <cassert>

auto testVtable () -> void {
  using Traits = ticket::internal::VariantTraits<int, long, bool>;
  static_assert(Traits::includes<int>() && !Traits::includes<float>());
  static_assert(Traits::indexOf<long>() == 1);
  static_assert(std::is_same_v<bool, Traits::NthType<2>>);
  int count = 0;
  int i = 233;
  long l = 19260817;
  bool b = false;
  auto f = [&] (auto &x) {
    switch (count++) {
      case 0: assert(x == i); break;
      case 1: assert(x == l); break;
      case 2: assert(x == b); break;
    }
  };
  using Vtable = Traits::Vtable<decltype(f)>;
  Vtable::visit(0, f, &i);
  Vtable::visit(1, f, &l);
  Vtable::visit(2, f, &b);
}

struct Int {
  int *value;
  Int (int, int v) : value(new int(v)) {}
  Int (const Int &other) : value(new int(*other.value)) {}
  Int () : value(new int(0)) {}
  ~Int () { delete value; }
};

auto testVariant () -> void {
  using T = ticket::Variant<Int, long, bool>;
  T v;
  v.visit([] (auto x) {
    bool good = std::is_same_v<decltype(x), Int>;
    assert(good);
    if constexpr (std::is_same_v<decltype(x), Int>) {
      assert(*x.value == 0);
    }
  });
  v = 19260817L;
  T v1 = v;
  assert(v.index() == 1 && v.is<long>());
  assert(v.get<bool>() == nullptr);
  assert(*v.get<long>() == 19260817L);
  assert(v1.index() == 1 && v.is<long>());
  assert(v1.get<bool>() == nullptr);
  assert(*v1.get<1>() == 19260817L);
}

auto testVariant2 () -> void {
  using T = ticket::Variant<long, Int>;
  T v;
  v.visit([] (auto x) {
    bool good = std::is_same_v<decltype(x), long>;
    assert(good);
    if constexpr (std::is_same_v<decltype(x), long>) {
      assert(x == 0);
    }
  });
  v = Int(2, 19260817);
  T v1 = v;
  assert(v.index() == 1 && v.is<Int>());
  assert(v.get<long>() == nullptr);
  assert(*(v.get<Int>()->value) == 19260817);
  assert(v1.index() == 1 && v.is<Int>());
  assert(v1.get<long>() == nullptr);
  assert(*(v1.get<1>()->value) == 19260817);
}

auto main () -> int {
  testVtable();
  testVariant();
  testVariant();
  testVariant2();
  return 0;
}
