#ifndef TICKET_LIB_LRU_CACHE_H_
#define TICKET_LIB_LRU_CACHE_H_

#include <cstring>

#include "hashmap.h"
#include "optional.h"
#include "utility.h"

namespace ticket {

/// A fixed-size cache with a least recently used policy.
template <typename Key, int kSize, typename BeforeDestroy>
class LruCache {
 private:
  struct WeightedKey;
 public:
  LruCache () = default;
  LruCache (const BeforeDestroy &callback) :
    callback_(callback) {}
  ~LruCache () {
    clear();
  }
  /// tries to obtain the value at the designated key.
  auto get (const Key &key) -> Optional<void *> {
    auto it = storage_.find(key);
    if (it == storage_.end()) return unit;
    auto buf = it->second.value;
    touch_(it);
    return buf;
  }
  /**
   * @brief upserts an cache entry.
   *
   * performs an insert if the key is not in the cache, or
   * an update if the key is in the cache.
   */
  auto upsert (
    const Key &key,
    const void *buf,
    int length,
    bool dirty = false
  ) -> void {
    auto it = storage_.find(key);
    if (it == storage_.end()) {
      // the key is not in the cache; insert.

      // is there enough space for a new entry?
      TICKET_ASSERT(storage_.size() <= kSize);
      if (storage_.size() == kSize) {
        // not enough space; clean the lru entry.
        auto willDelete = storage_.begin();
        auto &key = willDelete->first;
        auto &payload = willDelete->second;
        if (payload.dirty) {
          callback_(key, payload.value, payload.length);
        }
        delete[] payload.value;
        storage_.erase(willDelete);
      }
      TICKET_ASSERT(storage_.size() < kSize);

      // okay, we must have enough space here.
      storage_.insert({
        key,
        {
          allocate_(buf, length),
          length,
          dirty,
        }
      });

      return;
    } // if (it == storage_.end())

    // the key is in the cache. update the cache.
    auto &value = it->second.value;
    it->second.dirty = true;
    if (length != it->second.length) {
      delete[] value;
      value = allocate_(buf, length);
      it->second.length = length;
    } else {
      memcpy(value, buf, length);
    }
    touch_(it);
  }

  /// removes the key from the cache.
  auto remove (const Key &key) -> void {
    auto it = storage_.find(key);
    if (it == storage_.end()) return;
    auto &store = it->second;
    if (store.dirty) {
      callback_(key, store.value, store.length);
    }
    delete[] it->second.value;
    storage_.erase(it);
  }

  /// clears the cache.
  auto clear () -> void {
    for (auto &pair : storage_) {
      auto &store = pair.second;
      if (store.dirty) {
        callback_(pair.first, store.value, store.length);
      }
      delete[] store.value;
    }
    storage_.clear();
  }
 private:
  static_assert(kSize >= 2);
  struct Payload {
    char *value;
    int length;
    bool dirty;
  };
  HashMap<Key, Payload> storage_;
  BeforeDestroy callback_;

  template <typename Iterator>
  auto touch_ (Iterator it) -> void {
    auto key = it->first;
    auto payload = it->second;
    storage_.erase(it);
    storage_.insert({ key, payload });
  }

  auto allocate_ (const void *buf, int length) -> char * {
    char *copy = new char[length];
    memcpy(copy, buf, length);
    return copy;
  }
};

} // namespace ticket

#endif // TICKET_LIB_LRU_CACHE_H_
