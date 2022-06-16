#include "lru-cache.h"

#include <assert.h>
#include <string.h>

bool called = false;
auto handler (int key, char *buf, int length) -> void {
  called = true;
}

template <typename Key, int kSize, typename Callback>
using Cache = ticket::LruCache<Key, kSize, Callback>;

auto main () -> int {
  Cache<int, 4, decltype(&handler)> c{handler};
  assert(!c.get(0));
  char tmp[10];
  strncpy(tmp, "hello", sizeof(tmp));
  c.upsert(0, tmp, 6);
  c.upsert(1, tmp, 6);
  c.upsert(2, tmp, 6);
  c.upsert(2, tmp, 6);
  assert(strncmp((char *) *c.get(0), "hello", 6) == 0);
  strncpy(tmp, "world", sizeof(tmp));
  assert(strncmp((char *) *c.get(0), "hello", 6) == 0);
  c.upsert(0, tmp, 6);
  assert(strncmp((char *) *c.get(0), "world", 6) == 0);
  assert(strncmp((char *) *c.get(1), "hello", 6) == 0);
  c.upsert(0, tmp, 6);
  c.upsert(3, tmp, 6);
  assert(!called);
  c.upsert(4, tmp, 6);
  assert(called);
  called = false;
  assert(!c.get(2));
  c.remove(4);
  assert(!c.get(4));
  c.upsert(5, tmp, 6);
  assert(!called);
  assert(c.get(0) && c.get(1) && c.get(3));
  return 0;
}
