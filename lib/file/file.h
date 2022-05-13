// This file defines several basic file-based utilities.
#ifndef TICKET_LIB_FILE_FILE_H_
#define TICKET_LIB_FILE_FILE_H_

#include <cstring>
#include <fstream>

#include "hashmap.h"
#include "utility.h"
#include "exception.h"

/// File utilities
namespace ticket::file {

constexpr size_t kDefaultSzChunk = 4096;

/**
 * @brief A chunked file storage with manual garbage
 * collection.
 *
 * It is of chunk size of szChunk and has cache powered by
 * HashMap.
 */
template <typename Meta = Unit, size_t szChunk = kDefaultSzChunk>
class File {
 private:
  class Metadata;
 public:
  /**
   * @brief initializes the file at filename.
   *
   * it is not thread-safe.
   *
   * @param filename the file to open
   * @param initializer callback called on the creation of
   *   the file, when the file is empty.
   */
  template <typename Functor>
  File (const char *filename, const Functor &initializer) {
    init_(filename, initializer);
  }
  File (const char *filename) {
    init_(filename, [] {});
  }
  ~File () { clearCache(); }

  /// read n bytes at index into buf.
  auto get (void *buf, size_t index, size_t n) -> void {
    if (index != -1 && cache_.count(index) > 0) {
      memcpy(buf, cache_[index], n);
      return;
    }
    file_.seekg(offset_(index));
    file_.read((char *) buf, n);
    TICKET_ASSERT(file_.good());
    if (index != -1) putCache_(buf, index, n);
  }
  /// write n bytes at index from buf.
  auto set (const void *buf, size_t index, size_t n) -> void {
    if (index != -1) {
      // dirty check
      if (cache_.count(index) > 0 && memcmp(buf, cache_[index], n) == 0) return;
      putCache_(buf, index, n);
    }
    file_.seekp(offset_(index));
    file_.write((const char *) buf, n);
    TICKET_ASSERT(file_.good());
  }
  /// @returns the stored index of the object
  auto push (const void *buf, size_t n) -> size_t {
    Metadata meta = meta_();
    size_t id = meta.next;
    if (meta.hasNext) {
      Metadata nextMeta;
      get(&nextMeta, meta.next, sizeof(nextMeta));
      set(&nextMeta, -1, sizeof(nextMeta));
    } else {
      ++meta.next;
      set(&meta, -1, sizeof(meta));
    }
    set(buf, id, n);
    return id;
  }
  auto remove (size_t index) -> void {
    Metadata meta = meta_();
    set(&meta, index, sizeof(meta));
    Metadata newMeta(index, true);
    set(&newMeta, -1, sizeof(newMeta));
    if (cache_.count(index) > 0) delete[] cache_[index];
    cache_.erase(cache_.find(index));
  }

  /// gets user-provided metadata.
  auto getMeta () -> Meta {
    return meta_().user;
  }
  /// sets user-provided metadata.
  auto setMeta (const Meta &user) -> void {
    Metadata meta = meta_();
    meta.user = user;
    set(&meta, -1, sizeof(meta));
  }

  /// clears the cache.
  auto clearCache () -> void {
    for (const auto &[ _, ptr ] : cache_) delete[] ptr;
    cache_.clear();
  }

  /// clears file contents.
  auto truncate () -> void {
    Metadata meta(0, false);
    set(&meta, -1, sizeof(meta));
  }

 private:
  struct Metadata {
    size_t next;
    bool hasNext;
    Meta user;
    Metadata () = default;
    Metadata (size_t next, bool hasNext) : next(next), hasNext(hasNext) {}
  };
  static_assert(szChunk > sizeof(Metadata));

  template <typename Functor>
  auto init_ (const char *filename, const Functor &initializer) -> void {
    bool shouldCreate = false;
    auto testFile = fopen(filename, "r");
    if (testFile == nullptr) {
      if (errno != ENOENT) {
        throw IoException("Unable to open file");
      }
      shouldCreate = true;
    } else if (fclose(testFile)) {
      throw IoException("Unable to close file");
    }
    if (shouldCreate) {
      auto file = fopen(filename, "w+");
      if (file == nullptr) {
        throw IoException("Unable to create file");
      }
      if (fclose(file)) {
        throw IoException("Unable to close file when creating file");
      }
    }
    file_.open(filename);
    if (!file_.is_open() || !file_.good()) {
      throw IoException("Unable to open file");
    }
    if (shouldCreate) {
      truncate();
      initializer();
    }
  }

  auto meta_ () -> Metadata {
    Metadata retval;
    get(&retval, -1, sizeof(retval));
    return retval;
  }
  auto offset_ (size_t index) -> size_t {
    return (index + 1) * szChunk;
  }
  std::fstream file_;
  HashMap<size_t, char *> cache_;
  auto putCache_ (const void *buf, size_t index, size_t n) -> void {
    char *cache = new char[n];
    memcpy(cache, buf, n);
    if (cache_.count(index) > 0) delete[] cache_[index];
    cache_[index] = cache;
  }
};

/**
 * @brief an opinionated utility class wrapper for the
 * objects to be stored.
 *
 * it handles get, update, and push for the object.
 *
 * the base class needs to have a static char *filename.
 */
template <typename T, typename Meta = Unit>
class Managed : public T {
 public:
  /// The underlying file storage.
  static File<Meta, sizeof(T)> file;

  /**
   * @brief the unique immutable numeral identifier of the
   * object.
   *
   * this identifier would not change on update, but may be
   * reused when deleted.
   */
  auto id () const -> size_t { return id_; }

  /// gets the object at id in file.
  static auto get (size_t id) -> Managed {
    char buf[sizeof(Managed)];
    auto managed = reinterpret_cast<Managed *>(buf);
    auto unmanaged = static_cast<T *>(managed);
    file.get(unmanaged, id, sizeof(T));
    managed->id_ = id;
    return *managed;
  }
  /// hard deletes all objects.
  static auto truncate () -> void {
    file.truncate();
  }

  /**
   * @brief saves the object into the file.
   *
   * The object needs to be new, i.e. not saved before. To
   * update the object after a modification, use update().
   */
  auto save () -> void {
    TICKET_ASSERT(id_ == -1);
    id_ = file.push(static_cast<T *>(this), sizeof(T));
  }
  /// updates a modified object.
  auto update () -> void {
    TICKET_ASSERT(id_ != -1);
    file.set(static_cast<T *>(this), id_, sizeof(T));
  }
  /// removes the object from the file.
  auto destroy () -> void {
    TICKET_ASSERT(id_ != -1);
    file.remove(id_);
    id_ = -1;
  }
 private:
  size_t id_ = -1;
};

template <typename T, typename Meta>
File<Meta, sizeof(T)> Managed<T, Meta>::file { T::filename };

} // namespace ticket::file

#endif // TICKET_LIB_FILE_FILE_H_
