#include "lru-cache.h"

#include <assert.h>
#include <string.h>

template <typename Key, int kSize>
using Cache = ticket::LruCache<Key, kSize>;

auto main () -> int {
  Cache<int, 4> c;
  assert(!c.get(0));
  char tmp[10];
  strncpy(tmp, "hello", sizeof(tmp));
  assert(c.upsert(0, tmp, 6));
  assert(!c.upsert(0, tmp, 6));
  assert(!c.upsert(0, tmp, 6));
  assert(c.upsert(1, tmp, 6));
  assert(c.upsert(2, tmp, 6));
  assert(strncmp((char *) *c.get(0), "hello", 6) == 0);
  strncpy(tmp, "world", sizeof(tmp));
  assert(strncmp((char *) *c.get(0), "hello", 6) == 0);
  assert(c.upsert(0, tmp, 6));
  assert(strncmp((char *) *c.get(0), "world", 6) == 0);
  assert(strncmp((char *) *c.get(1), "hello", 6) == 0);
  assert(!c.upsert(0, tmp, 6));
  assert(c.upsert(3, tmp, 6));
  assert(c.upsert(4, tmp, 6));
  assert(!c.get(2));
  c.remove(4);
  assert(!c.get(4));
  assert(c.upsert(5, tmp, 6));
  assert(c.get(0) && c.get(1) && c.get(3));
  return 0;
}
