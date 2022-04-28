#include "result.h"

#include <cassert>

#include "exception.h"

using Res = ticket::Result<int, ticket::Exception>;

auto err () -> Res {
  return ticket::Exception("Hello World");
}
auto succ () -> Res {
  return 1926;
}

auto rethrow () -> ticket::Result<Res, ticket::Exception> {
  auto res = err();
  if (auto err = res.error()) {
    return *err;
  }
  assert(false);
}

auto main () -> int {
  auto resErr = rethrow();
  if (auto err = resErr.error()) {
    assert(!resErr.success());
  } else {
    assert(false);
  }
  auto resSucc = succ();
  if (auto err = resSucc.error()) {
    assert(false);
  } else {
    assert(resSucc.success());
    assert(resSucc.result() == 1926);
  }
  return 0;
}
