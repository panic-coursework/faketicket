#ifndef TICKET_LIB_FILE_INDEX_H_
#define TICKET_LIB_FILE_INDEX_H_

#include "file/bptree.h"
#include "file/varchar.h"
#include "optional.h"
#include "vector.h"

namespace ticket::file {

/**
 * @brief Class representing an index file.
 *
 * The Index maps Key to Model's numerical identifier, and
 * provides methods to directly retrieve model objects from
 * data files.
 *
 * Model needs to be a subclass of ManagedObject.
 */
template <typename Key, typename Model>
class Index {
 public:
  /**
   * @brief initializes the index.
   * @param ptr the member pointer of the key.
   * @param filename file to store the key.
   * @param datafile the main file where data is stored.
   */
  Index (Key Model::*ptr, const char *filename)
    : ptr_(ptr), tree_(filename) {}
  /// inserts an object into the index.
  auto insert (const Model &model) -> void {
    tree_.insert(model.*ptr_, model.id());
  }
  /// removes an object from the index.
  auto remove (const Model &model) -> void {
    tree_.remove(model.*ptr_, model.id());
  }
  /// finds one Model in the index.
  auto findOne (const Key &key) -> Optional<Model> {
    auto id = tree_.findOne(key);
    if (!id) return unit;
    return Model::get(*id);
  }
  /// finds one identifier in the index.
  auto findOneId (const Key &key) -> Optional<int> {
    return tree_.findOne(key);
  }
  /// finds all Models of the given key in the index.
  auto findMany (const Key &key) -> Vector<Model> {
    Vector<Model> res;
    auto ids = tree_.findMany(key);
    if (ids.size() > 0) res.reserve(ids.size());
    for (auto id : ids) {
      res.push_back(Model::get(id));
    }
    return res;
  }
  /// finds all IDs of the given keys in the index.
  auto findManyId (const Key &key) -> Vector<int> {
    return tree_.findMany(key);
  }
 private:
  Key Model::*ptr_;
  BpTree<Key, int> tree_;
};

/**
 * @brief Specialization of Index on Varchar.
 *
 * It makes use of hashes to speed up the process.
 */
template <size_t maxLength, typename Model>
class Index<Varchar<maxLength>, Model> {
 private:
  using Key = Varchar<maxLength>;
 public:
  /**
   * @brief initializes the index.
   * @param ptr the member pointer of the key.
   * @param filename file to store the key.
   * @param datafile the main file where data is stored.
   */
  Index (Key Model::*ptr, const char *filename)
    : ptr_(ptr), tree_(filename) {}
  /// inserts an object into the index.
  auto insert (const Model &model) -> void {
    tree_.insert(model.*ptr_.hash(), model.id());
  }
  /// removes an object from the index.
  auto remove (const Model &model) -> void {
    tree_.remove(model.*ptr_.hash(), model.id());
  }
  /// finds one Model in the index.
  auto findOne (const Key &key) -> Optional<Model> {
    auto id = tree_.findOne(key.hash());
    if (!id) return unit;
    return Model::get(*id);
  }
  /// finds one identifier in the index.
  auto findOneId (const Key &key) -> Optional<int> {
    return tree_.findOne(key.hash());
  }
  /// finds all Models of the given key in the index.
  auto findMany (const Key &key) -> Vector<Model> {
    Vector<Model> res;
    auto ids = tree_.findMany(key.hash());
    if (ids.size() > 0) res.reserve(ids.size());
    for (auto id : ids) {
      res.push_back(Model::get(id));
    }
    return res;
  }
  /// finds all IDs of the given keys in the index.
  auto findManyId (const Key &key) -> Vector<int> {
    return tree_.findMany(key.hash());
  }
 private:
  Key Model::*ptr_;
  BpTree<size_t, int> tree_;
};

} // namespace ticket::file

#endif // TICKET_LIB_FILE_INDEX_H_
