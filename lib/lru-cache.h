#ifndef TICKET_LIB_LRU_CACHE_H_
#define TICKET_LIB_LRU_CACHE_H_

#include <cstring>

#include "hashmap.h"
#include "map.h"
#include "optional.h"
#include "utility.h"

namespace ticket {

/// A fixed-size cache with a least recently used policy.
template <typename Key, int kSize>
class LruCache {
 private:
  struct WeightedKey;
 public:
  ~LruCache () {
    clear();
  }
  /// tries to obtain the value at the designated key.
  auto get (const Key &key) -> Optional<void *> {
    auto it = storage_.find(key);
    if (it == storage_.end()) return unit;
    touch_(it);
    return it->second.value;
  }
  /**
   * @brief upserts an cache entry.
   * @returns true if the cache state has changed.
   *
   * performs an insert if the key is not in the cache, or
   * an update if the key is in the cache.
   */
  auto upsert (const Key &key, const void *buf, int length)
    -> bool {
    auto it = storage_.find(key);
    if (it == storage_.end()) {
      // the key is not in the cache; insert.

      // is there enough space for a new entry?
      TICKET_ASSERT(index_.size() <= kSize);
      TICKET_ASSERT(index_.size() == storage_.size());
      if (index_.size() == kSize) {
        // not enough space; clean the lru entry.
        auto willDelete = index_.begin();
        auto &key = willDelete->second;

        auto willDeleteStorage = storage_.find(key);
        auto value = willDeleteStorage->second.value;
        delete[] value;

        storage_.erase(willDeleteStorage);
        index_.erase(willDelete);
      }
      TICKET_ASSERT(index_.size() < kSize);
      TICKET_ASSERT(index_.size() == storage_.size());

      // okay, we must have enough space here.
      ++currentTime_;
      index_[currentTime_] = key;
      storage_[key] =
        { currentTime_, (char *) allocate_(buf, length) };

      // the cache has changed.
      return true;
    } // if (it == storage_.end())

    // the key is in the cache. check, then update if needed
    touch_(it);
    auto &value = it->second.value;
    if (memcmp(buf, value, length) == 0) {
      // content is identical, no update needed.
      return false;
    }
    // content is different, update needed.
    delete[] value;
    value = (char *) allocate_(buf, length);
    return true;
  }

  /// removes the key from the cache.
  auto remove (const Key &key) -> void {
    auto it = storage_.find(key);
    if (it == storage_.end()) return;
    delete[] it->second.value;
    index_.erase(index_.find(it->second.accessTime));
    storage_.erase(it);
  }

  /// clears the cache.
  auto clear () -> void {
    for (auto &pair : storage_) delete[] pair.second.value;
    index_.clear();
    storage_.clear();
  }
 private:
  static_assert(kSize >= 2);
  struct WeightedValue {
    int accessTime;
    char *value;
  };
  int currentTime_ = 0;
  Map<int, Key> index_;
  HashMap<Key, WeightedValue> storage_;

  template <typename Iterator>
  auto touch_ (Iterator it) -> void {
    const auto &key = it->first;
    auto &value = it->second;
    index_.erase(index_.find(value.accessTime));
    value.accessTime = ++currentTime_;
    index_[value.accessTime] = key;
  }

  auto allocate_ (const void *buf, int length) -> void * {
    char *copy = new char[length];
    memcpy(copy, buf, length);
    return copy;
  }
};

} // namespace ticket

#endif // TICKET_LIB_LRU_CACHE_H_
