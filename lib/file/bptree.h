#ifndef TICKET_LIB_FILE_BPTREE_H_
#define TICKET_LIB_FILE_BPTREE_H_

#include <cstring>

#include "algorithm.h"
#include "file/array.h"
#include "file/file.h"
#include "file/internal/unmanaged-object.h"
#include "file/set.h"
#include "optional.h"
#include "utility.h"
#include "vector.h"

#ifdef TICKET_DEBUG
#include <iostream>
#endif

namespace ticket::file {

/**
 * @brief an implementation of the B+ tree.
 *
 * it stores key and value together in order to support
 * duplicate keys.
 *
 * constraints: KeyType and ValueType need to be comparable.
 */
template <
  typename KeyType,
  typename ValueType,
  typename CmpKey = Less<>,
  typename CmpValue = Less<>,
  typename Meta = Unit,
  size_t szChunk = kDefaultSzChunk
>
class BpTree {
 private:
  struct Node;
 public:
  /// constructs a B+ tree on the given file.
  BpTree (const char *filename) : file_(filename, [this] () { this->init_(); }) {}
  /**
   * @brief inserts a key-value pair into the tree.
   *
   * duplicate keys is supported, though duplicate key-value
   * pair leads to undefined behavior, and may lead to an
   * invalid tree.
   */
  auto insert (const KeyType &key, const ValueType &value) -> void {
    Node root = Node::root(*this);
    insert_({ .key = key, .value = value }, root);
    if (root.shouldSplit()) split_(root, root, 0);
    root.update();
  }
  /**
   * @brief removes a key-value pair from the tree.
   *
   * you must ensure that the entry is indeed in the tree.
   * removing an nonexistent entry may lead to an invalid
   * tree.
   */
  auto remove (const KeyType &key, const ValueType &value) -> void {
    Node root = Node::root(*this);
    remove_({ .key = key, .value = value }, root);
    if (root.shouldMerge()) merge_(root, root, 0);
    root.update();
  }
  /// finds the first entry with the given key.
  auto findOne (const KeyType &key) -> Optional<ValueType> {
    return findOne_(key, Node::root(*this));
  }
  /// finds all entries with the given key.
  auto findMany (const KeyType &key) -> Vector<ValueType> {
    return findMany_(key, Node::root(*this));
  }
  /// finds all entries.
  auto findAll () -> Vector<ticket::Pair<KeyType, ValueType>> {
    return findAll_(Node::root(*this));
  }
  /// checks if the given key-value pair exists in the tree.
  auto includes (const KeyType &key, const ValueType &value) -> bool {
    return includes_({ .key = key, .value = value }, Node::root(*this));
  }
  /// checks if the tree is empty.
  auto empty () -> bool {
    return Node::root(*this).length() == 0;
  }

  /// gets user-provided metadata.
  auto getMeta () -> Meta {
    return file_.getMeta();
  }
  /// sets user-provided metadata.
  auto setMeta (const Meta &meta) -> void {
    return file_.setMeta(meta);
  }

  /**
   * @brief clears the cache of the underlying file.
   *
   * you may need to call this method periodically to avoid
   * using up too much memory.
   */
  auto clearCache () -> void { file_.clearCache(); }
  /// hard deletes all entries in the tree.
  auto truncate () -> void {
    file_.truncate();
    init_();
  }

#ifdef TICKET_DEBUG
  auto print () -> void { print_(Node::root(*this)); }
#endif

 private:
  File<Meta, szChunk> file_;
  CmpKey cmpKey_;
  CmpValue cmpValue_;

  // data structures
  /// store key and value together to support dupe keys. this is the structure that is actually stored.
  struct Pair {
    KeyType key;
    ValueType value;
    auto operator< (const Pair &that) const -> bool {
      CmpKey cmpKey_;
      CmpValue cmpValue_;
      if (!cmpKey_.equals(key, that.key)) return cmpKey_.lt(key, that.key);
      return cmpValue_.lt(value, that.value);
    }
  };
  /// compares a Payload and a KeyType that key alone is less than all payloads with this key
  class KeyComparatorLess_ {
   public:
    auto operator() (const Pair &lhs, const KeyType &rhs) -> bool {
      return cmpKey_.lt(lhs.key, rhs);
    }
    auto operator() (const KeyType &lhs, const Pair &rhs) -> bool {
      return cmpKey_.geq(rhs.key, lhs);
    }
   private:
    CmpKey cmpKey_;
    CmpValue cmpValue_;
  };

  using NodeId = unsigned int;
  // ROOT and INTERMEDIATE nodes are index nodes
  enum NodeType { kRoot, kIntermediate, kRecord };
  // if k > kLengthMax, there must be an overflow.
  static constexpr size_t kLengthMax = 18446744073709000000ULL;
  struct IndexPayload {
    static constexpr size_t k = (szChunk - 2 * sizeof(NodeId)) / (sizeof(NodeId) + sizeof(Pair)) / 2 - 1;
    static_assert(k >= 2 && k < kLengthMax);
    bool leaf = false;
    /// for leaf nodes, childs are the indices of data nodes.
    Array<NodeId, 2 * k> children;
    Set<Pair, 2 * k> splits;
  };
  struct RecordPayload {
    static constexpr size_t l = (szChunk - 3 * sizeof(NodeId)) / sizeof(Pair) / 2 - 1;
    static_assert(l >= 2 && l < kLengthMax);
    NodeId prev = 0;
    NodeId next = 0;
    Set<Pair, 2 * l> entries;
  };
  union NodePayload {
    IndexPayload index;
    RecordPayload record;
    NodePayload () {} // NOLINT
  };
  struct Node : public internal::UnmanagedObject<Node, Meta, szChunk> {
    char _start[0];
    NodeType type;
    NodePayload payload;
    char _end[0];
    static_assert(sizeof(NodeType) + sizeof(NodePayload) <= szChunk);

    // dynamically type-safe accessors
    auto leaf () -> bool & { TICKET_ASSERT(type != kRecord); return payload.index.leaf; }
    auto children () -> Array<NodeId, 2 * IndexPayload::k> & { TICKET_ASSERT(type != kRecord); return payload.index.children; }
    auto splits () -> Set<Pair, 2 * IndexPayload::k> & { TICKET_ASSERT(type != kRecord); return payload.index.splits; }
    auto prev () -> NodeId & { TICKET_ASSERT(type == kRecord); return payload.record.prev; }
    auto next () -> NodeId & { TICKET_ASSERT(type == kRecord); return payload.record.next; }
    auto entries () -> Set<Pair, 2 * RecordPayload::l> & { TICKET_ASSERT(type == kRecord); return payload.record.entries; }

    Node (BpTree &tree, NodeType type) : internal::UnmanagedObject<Node, Meta, szChunk>(tree.file_), type(type) {
      if (type == kRecord) {
        new(&payload.record) RecordPayload;
      } else {
        new(&payload.index) IndexPayload;
      }
    }
    ~Node () {
      if (type == kRecord) {
        payload.record.~RecordPayload();
      } else {
        payload.index.~IndexPayload();
      }
    }

    static auto root (BpTree &tree) -> Node { return Node::get(tree.file_, 0); }

    auto halfLimit () -> size_t {
      return type == kRecord ? RecordPayload::l : IndexPayload::k;
    }
    auto length () -> size_t {
      return type == kRecord ? payload.record.entries.length : payload.index.children.length;
    }
    auto shouldSplit () -> bool { return length() == 2 * halfLimit(); }
    auto shouldMerge () -> bool { return length() < halfLimit(); }
    auto lowerBound () -> Pair {
      return type == kRecord ? payload.record.entries[0] : payload.index.splits[0];
    }
  };

  // helper functions
  auto ixInsert_ (const Pair &entry, Node &node) -> size_t {
    TICKET_ASSERT(node.type != kRecord);
    auto &splits = node.splits();
    size_t ix = upperBound(splits.content, splits.content + splits.length, entry) - splits.content;
    return ix == 0 ? ix : ix - 1;
  }
  auto splitRoot_ (Node &node) -> void {
    Node left(*this, kIntermediate), right(*this, kIntermediate);

    // copy children and splits
    left.children().copyFrom(node.children(), 0, 0, IndexPayload::k);
    left.splits().copyFrom(node.splits(), 0, 0, IndexPayload::k);
    right.children().copyFrom(node.children(), IndexPayload::k, 0, IndexPayload::k);
    right.splits().copyFrom(node.splits(), IndexPayload::k, 0, IndexPayload::k);
    left.children().length = left.splits().length = right.children().length = right.splits().length = IndexPayload::k;

    // set misc properties and save
    left.leaf() = right.leaf() = node.leaf();
    node.leaf() = false;
    left.save();
    right.save();

    // initiate the new root node
    node.children().clear();
    node.children().insert(left.id(), 0);
    node.children().insert(right.id(), 1);
    node.splits().clear();
    node.splits().insert(left.lowerBound());
    node.splits().insert(right.lowerBound());
  }
  auto split_ (Node &node, Node &parent, size_t ixChild) -> void {
    TICKET_ASSERT(node.shouldSplit());
#ifdef TICKET_DEBUG_BPTREE
    ;// std::cerr << "[Split] " << node.id() << " (parent " << parent.id() << ")" << std::endl;
#endif
    if (node.type == kRoot) {
      // the split of the root node is a bit different from other nodes. it produces two extra subnodes.
      splitRoot_(node);
      return;
    }
    TICKET_ASSERT(node.type != kRoot);

    // create a new next node
    Node next(*this, node.type);
    if (node.type == kIntermediate) {
      next.children().copyFrom(node.children(), IndexPayload::k, 0, IndexPayload::k);
      next.splits().copyFrom(node.splits(), IndexPayload::k, 0, IndexPayload::k);
      node.children().length = node.splits().length = next.children().length = next.splits().length = IndexPayload::k;
      next.leaf() = node.leaf();
      next.save();
    } else {
      TICKET_ASSERT(node.type == kRecord);
      next.next() = node.next();
      next.prev() = node.id();
      memmove(
        next.entries().content,
        &node.entries().content[RecordPayload::l],
        RecordPayload::l * sizeof(node.entries()[0])
      );
      next.entries().length = node.entries().length = RecordPayload::l;
      next.save();
      if (next.next() != 0) {
        Node nextnext = Node::get(file_, next.next());
        nextnext.prev() = next.id();
        nextnext.update();
      }
      node.next() = next.id();
    }

    // update the parent node
    parent.children().insert(next.id(), ixChild + 1);
    parent.splits().insert(next.lowerBound());
  }

  template <typename A, typename B>
  static auto unshift_ (A &to, B &from, size_t k) -> void {
    // we now have [b[0],...,b[k-1]] and [a[0]...a[k-2]], want a -> [b[0],...,b[k-1],a[0],...,a[k-2]]
    to.copyFrom(to, 0, k, k - 1);
    to.copyFrom(from, 0, 0, k);
    to.length += from.length;
    from.length = 0;
  }
  template <typename A, typename B>
  static auto push_ (A &to, B &from, size_t k) -> void {
    to.copyFrom(from, 0, k - 1, k);
    to.length += from.length;
    from.length = 0;
  }
  auto merge_ (Node &node, Node &parent, size_t ixChild) -> void {
    TICKET_ASSERT(node.shouldMerge());
#ifdef TICKET_DEBUG_BPTREE
    ;// std::cerr << "[Merge] " << node.id() << " (parent " << parent.id() << ")" << std::endl;
#endif
    if (node.type == kRoot) {
      if (node.length() > 1 || node.leaf()) return;
      Node onlyChild = Node::get(file_, node.children()[0]);
      memcpy(&node, &onlyChild, sizeof(node));
      node.type = kRoot;
      return;
    }
    const bool hasPrev = ixChild != 0;
    const bool hasNext = ixChild != parent.children().length - 1;
    if (!hasNext) {
      // don't do anything to the only data node.
      if (!hasPrev && node.type == kRecord) return;
      // all index nodes has at least 2 child nodes, except for the root node.
      TICKET_ASSERT(hasPrev);
      Node prev = Node::get(file_, parent.children()[ixChild - 1]);
      if (prev.length() > prev.halfLimit()) {
        if (node.type == kRecord) {
          node.entries().insert(prev.entries().pop());
        } else {
          node.children().unshift(prev.children().pop());
          node.splits().insert(prev.splits().pop());
        }
        prev.update();
        parent.splits()[ixChild] = node.lowerBound();
        return;
      }
      TICKET_ASSERT(prev.length() == prev.halfLimit());

      if (node.type == kRecord) {
        unshift_(node.entries(), prev.entries(), RecordPayload::l);
        if (prev.prev() != 0) {
          Node prevprev = Node::get(file_, prev.prev());
          prevprev.next() = node.id();
          prevprev.update();
        }
        node.prev() = prev.prev();
      } else {
        TICKET_ASSERT(node.type == kIntermediate);
        unshift_(node.children(), prev.children(), IndexPayload::k);
        unshift_(node.splits(), prev.splits(), IndexPayload::k);
      }
      parent.splits()[ixChild] = node.lowerBound();
      parent.children().removeAt(ixChild - 1);
      parent.splits().removeAt(ixChild - 1);
      prev.destroy();
      return;
    }
    TICKET_ASSERT(hasNext);

    // FIXME: remove dupe code here
    Node next = Node::get(file_, parent.children()[ixChild + 1]);
    if (next.length() > next.halfLimit()) {
      if (node.type == kRecord) {
        node.entries().insert(next.entries().shift());
      } else {
        node.children().push(next.children().shift());
        node.splits().insert(next.splits().shift());
      }
      next.update();
      parent.splits()[ixChild + 1] = next.lowerBound();
      return;
    }
    TICKET_ASSERT(next.length() == next.halfLimit());

    if (node.type == kRecord) {
      push_(node.entries(), next.entries(), RecordPayload::l);
      if (next.next() != 0) {
        Node nextnext = Node::get(file_, next.next());
        nextnext.prev() = node.id();
        nextnext.update();
      }
      node.next() = next.next();
    } else {
      TICKET_ASSERT(node.type == kIntermediate);
      push_(node.children(), next.children(), IndexPayload::k);
      push_(node.splits(), next.splits(), IndexPayload::k);
    }

    parent.children().removeAt(ixChild + 1);
    parent.splits().removeAt(ixChild + 1);
    next.destroy();
  }

  // FIXME: lengthy function name
  auto addValuesToVectorForAllKeyFrom_ (Vector<ValueType> &vec, const KeyType &key, Node node, int first) -> void {
    // we need to declare i outside to see if we have advanced to the last element
    int i = first;
    for (; i < node.length() && cmpKey_.equals(node.entries()[i].key, key); ++i) vec.push_back(node.entries()[i].value);
    if (i == node.length() && node.next() != 0) addValuesToVectorForAllKeyFrom_(vec, key, Node::get(file_, node.next()), 0);
  }
  auto addEntriesToVector_ (Vector<ticket::Pair<KeyType, ValueType>> &vec, Node node) -> void {
    for (int i = 0; i < node.length(); ++i) vec.emplace_back(node.entries()[i].key, node.entries()[i].value);
    if (node.next() != 0) addEntriesToVector_(vec, Node::get(file_, node.next()));
  }
  auto findFirstChildWithKey_ (const KeyType &key, Node &node) -> ticket::Pair<Node, Optional<Node>> {
    TICKET_ASSERT(node.type != kRecord);
    size_t ixGreater = upperBound(
      node.splits().content,
      node.splits().content + node.length(),
      key,
      Less<KeyComparatorLess_>()
    ) - node.splits().content;
    bool hasCdr = ixGreater < node.length() && cmpKey_.equals(node.splits()[ixGreater].key, key);
    auto cdr = hasCdr ? Optional<Node>(Node::get(file_, node.children()[ixGreater])) : Optional<Node>(unit);
    size_t ix = ixGreater == 0 ? ixGreater : ixGreater - 1;
    return { Node::get(file_, node.children()[ix]), cdr };
  }

  // operation functions
  auto insert_ (const Pair &entry, Node &node) -> void {
    if (node.type == kRecord) {
      node.entries().insert(entry);
      TICKET_ASSERT(node.entries().length <= 2 * RecordPayload::l);
      return;
    }
    // if this is the first entry of the root, go create a record node.
    if (node.children().length == 0) {
      TICKET_ASSERT(node.type == kRoot);
      TICKET_ASSERT(node.leaf());
      Node child(*this, kRecord);
      child.entries().insert(entry);
      child.save();
      node.children().push(child.id());
      node.splits().insert(entry);
      return;
    }
    size_t ix = ixInsert_(entry, node);
    if (entry < node.splits()[ix]) node.splits()[ix] = entry;
    Node nodeToInsert = Node::get(file_, node.children()[ix]);
    insert_(entry, nodeToInsert);
    node.splits()[ix] = nodeToInsert.lowerBound();
    if (nodeToInsert.shouldSplit()) split_(nodeToInsert, node, ix);
    nodeToInsert.update();
  }
  auto remove_ (const Pair &entry, Node &node) -> void {
    if (node.type == kRecord) {
      node.entries().remove(entry);
      return;
    }
    size_t ix = ixInsert_(entry, node);
    Node child = Node::get(file_, node.children()[ix]);
    remove_(entry, child);
    if (child.length() == 0) {
      TICKET_ASSERT(node.type == kRoot);
      TICKET_ASSERT(child.type == kRecord);
      child.destroy();
      node.children().clear();
      node.splits().clear();
      return;
    }
    node.splits()[ix] = child.lowerBound();
    if (child.shouldMerge()) merge_(child, node, ix);
    child.update();
  }
  auto findOne_ (const KeyType &key, Node node) -> Optional<ValueType> {
    if (node.type != kRecord) {
      if (node.length() == 0) return unit;
      auto [ car, cdr ] = findFirstChildWithKey_(key, node);
      if (!cdr) return findOne_(key, car);
      auto res = findOne_(key, car);
      if (res) return res;
      return findOne_(key, *cdr);
    }
    size_t ix = upperBound(
      node.entries().content,
      node.entries().content + node.length(),
      key,
      Less<KeyComparatorLess_>()
    ) - node.entries().content;
    if (ix >= node.length()) return unit;
    Pair entry = node.entries()[ix];
    if (!cmpKey_.equals(entry.key, key)) return unit;
    return entry.value;
  }
  auto includes_ (const Pair &entry, Node node) -> bool {
    if (node.type == kRecord) return node.entries().includes(entry);
    if (node.length() == 0) return false;
    return includes_(entry, Node::get(file_, node.children()[ixInsert_(entry, node)]));
  }
  auto findMany_ (const KeyType &key, Node node) -> Vector<ValueType> {
    if (node.type != kRecord) {
      if (node.length() == 0) return {};
      auto [ car, cdr ] = findFirstChildWithKey_(key, node);
      if (!cdr) return findMany_(key, car);
      Vector<ValueType> res = findMany_(key, car);
      if (!res.empty()) return res;
      return findMany_(key, *cdr);
    }
    size_t ix = upperBound(
      node.entries().content,
      node.entries().content + node.length(),
      key,
      Less<KeyComparatorLess_>()
    ) - node.entries().content;
    if (ix >= node.length()) return {};
    Vector<ValueType> res;
    addValuesToVectorForAllKeyFrom_(res, key, node, ix);
    return res;
  }
  auto findAll_ (Node node) -> Vector<ticket::Pair<KeyType, ValueType>> {
    if (node.type != kRecord) {
      if (node.length() == 0) return {};
      return findAll_(Node::get(file_, node.children()[0]));
    }
    Vector<ticket::Pair<KeyType, ValueType>> res;
    addEntriesToVector_(res, node);
    return res;
  }
  auto init_ () -> void {
    Node root(*this, kRoot);
    root.leaf() = true;
    root.save();
    TICKET_ASSERT(root.id() == 0);
  }
#ifdef TICKET_DEBUG
  auto print_ (Node node) -> void {
    if (node.type == kRecord) {
      ;// std::cerr << "[Record " << node.id() << " (" << node.length() << "/" << 2 * RecordPayload::l - 1 << ")]";
      for (int i = 0; i < node.length(); ++i) ;// std::cerr << " (" << std::string(node.entries()[i].key) << ", " << node.entries()[i].value << ")";
      ;// std::cerr << std::endl;
      return;
    }
    ;// std::cerr << "[Node " << node.id() << " (" << node.length() << "/" << 2 * IndexPayload::k - 1 << ")" << (node.leaf() ? " leaf" : "") << "]";
    for (int i = 0; i < node.length(); ++i) ;// std::cerr << " (" << std::string(node.splits()[i].key) << ", " << node.splits()[i].value << ") " << node.children()[i];
    ;// std::cerr << std::endl;
    for (int i = 0; i < node.length(); ++i) print_(Node::get(file_, node.children()[i]));
  }
#endif
};

} // namespace ticket::file

#endif // TICKET_LIB_FILE_BPTREE_H_
