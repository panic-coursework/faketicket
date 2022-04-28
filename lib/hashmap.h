#ifndef TICKET_LIB_HASHMAP_H_
#define TICKET_LIB_HASHMAP_H_

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>

#include "exception.h"
#include "utility.h"

#ifdef DEBUG
#include <iostream>
#endif

namespace ticket {

static constexpr unsigned long long pow2[] = { 1ULL << 0, 1ULL << 1, 1ULL << 2, 1ULL << 3, 1ULL << 4, 1ULL << 5, 1ULL << 6, 1ULL << 7, 1ULL << 8, 1ULL << 9, 1ULL << 10, 1ULL << 11, 1ULL << 12, 1ULL << 13, 1ULL << 14, 1ULL << 15, 1ULL << 16, 1ULL << 17, 1ULL << 18, 1ULL << 19, 1ULL << 20, 1ULL << 21, 1ULL << 22, 1ULL << 23, 1ULL << 24, 1ULL << 25, 1ULL << 26, 1ULL << 27, 1ULL << 28, 1ULL << 29, 1ULL << 30, 1ULL << 31, 1ULL << 32, 1ULL << 33, 1ULL << 34, 1ULL << 35, 1ULL << 36, 1ULL << 37, 1ULL << 38, 1ULL << 39, 1ULL << 40, 1ULL << 41, 1ULL << 42, 1ULL << 43, 1ULL << 44, 1ULL << 45, 1ULL << 46, 1ULL << 47, 1ULL << 48, 1ULL << 49, 1ULL << 50, 1ULL << 51, 1ULL << 52, 1ULL << 53, 1ULL << 54, 1ULL << 55, 1ULL << 56, 1ULL << 57, 1ULL << 58, 1ULL << 59, 1ULL << 60, 1ULL << 61, 1ULL << 62, 1ULL << 63 };
// mask = x => (1n << BigInt(x)) - 1n
// console.log(Array.from(Array(64).keys()).map(mask).join(', '))
static constexpr unsigned long long mask[] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 1073741823, 2147483647, 4294967295, 8589934591, 17179869183, 34359738367, 68719476735, 137438953471, 274877906943, 549755813887, 1099511627775, 2199023255551, 4398046511103, 8796093022207, 17592186044415, 35184372088831, 70368744177663, 140737488355327, 281474976710655, 562949953421311, 1125899906842623, 2251799813685247, 4503599627370495, 9007199254740991, 18014398509481983, 36028797018963967, 72057594037927935, 144115188075855871, 288230376151711743, 576460752303423487, 1152921504606846975, 2305843009213693951, 4611686018427387903, 9223372036854775807 };

inline constexpr unsigned rehashMagic1 = 1162192769;
inline constexpr unsigned rehashMagic2 = 3717357010;
template <typename T>
auto rehash (T value) -> unsigned {
  if ((value & pow2[2]) == 0) return value ^ rehashMagic1;
  return value ^ rehashMagic2;
}

/**
 * In HashMap, iteration ordering is differ from map,
 * which is the order in which keys were inserted into the map.
 * You should maintain a doubly-linked list running through all
 * of its entries to keep the correct iteration order.
 *
 * Note that insertion order is not affected if a key is re-inserted
 * into the map.
 */
template <
  typename Key,
  typename Value,
  typename Hash = std::hash<Key>,
  typename Equal = std::equal_to<Key>
> class HashMap {
 private:
  struct ListNode;
  struct Node;
 public:
  using value_type = Pair<const Key, Value>;

  class const_iterator;
  class iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = HashMap::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::output_iterator_tag;

    iterator () = default;
    iterator (ListNode *node, HashMap *home) : node_(node), home_(home) {}
    auto operator++ (int) -> iterator {
      if (node_ == &home_->pivot_) throw Exception("invalid state");
      auto node = node_;
      node_ = node_->next_;
      return { node, home_ };
    }
    auto operator++ () -> iterator & {
      if (node_ == &home_->pivot_) throw Exception("invalid state");
      node_ = node_->next_;
      return *this;
    }
    auto operator-- (int) -> iterator {
      if (node_ == home_->pivot_.next_) throw Exception("invalid state");
      auto node = node_;
      node_ = node_->prev_;
      return { node, home_ };
    }
    auto operator-- () -> iterator & {
      if (node_ == home_->pivot_.next_) throw Exception("invalid state");
      node_ = node_->prev_;
      return *this;
    }
    auto operator* () const -> reference {
      return node_->self->value;
    }
    auto operator== (const iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator== (const const_iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator!= (const iterator &rhs) const -> bool {
      return !(*this == rhs);
    }
    auto operator!= (const const_iterator &rhs) const -> bool {
      return !(*this == rhs);
    }
    auto operator-> () const noexcept -> pointer {
      return &**this;
    }
   private:
    ListNode *node_;
    HashMap *home_;
    friend class const_iterator;
    friend class HashMap;
  };

  class const_iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = const HashMap::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::output_iterator_tag;

    const_iterator () = default;
    const_iterator (const ListNode *node, const HashMap *home) : node_(node), home_(home) {}
    const_iterator (const iterator &other) : node_(other.node_), home_(other.home_) {}
    auto operator++ (int) -> const_iterator {
      if (node_ == &home_->pivot_) throw Exception("invalid state");
      auto node = node_;
      node_ = node_->next_;
      return { node, home_ };
    }
    auto operator++ () -> const_iterator & {
      if (node_ == &home_->pivot_) throw Exception("invalid state");
      node_ = node_->next_;
      return *this;
    }
    auto operator-- (int) -> const_iterator {
      if (node_ == home_->pivot_.next_) throw Exception("invalid state");
      auto node = node_;
      node_ = node_->prev_;
      return { node, home_ };
    }
    auto operator-- () -> const_iterator & {
      if (node_ == home_->pivot_.next_) throw Exception("invalid state");
      node_ = node_->prev_;
      return *this;
    }
    auto operator* () const -> reference {
      return node_->self->value;
    }
    auto operator== (const iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator== (const const_iterator &rhs) const -> bool {
      return node_ == rhs.node_;
    }
    auto operator!= (const iterator &rhs) const -> bool {
      return !(*this == rhs);
    }
    auto operator!= (const const_iterator &rhs) const -> bool {
      return !(*this == rhs);
    }
    auto operator-> () const noexcept -> pointer {
      return &**this;
    }
   private:
    const ListNode *node_;
    const HashMap *home_;
    friend class iterator;
    friend class HashMap;
  };

  HashMap () = default;
  HashMap (const HashMap &other) { *this = other; }
  auto operator= (const HashMap &other) -> HashMap & {
    if (this == &other) return *this;
    clear();
    capacity_ = other.capacity_;
    size_ = other.size_;
    store_ = new ListNode[pow2[capacity_]];
    const ListNode *node = &other.pivot_;
    for (int i = 0; i < size_; ++i) {
      node = node->next_;
      Node *newNode = new Node(*(node->self));
      int ix = newNode->hash & mask[capacity_];
      newNode->hashList.insertBefore(&store_[ix]);
      newNode->iteratorList.insertBefore(&pivot_);
    }
    return *this;
  }
  ~HashMap () {
    destroy_();
  }

  /**
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  auto at (const Key &key) -> Value & {
    auto it = find(key);
    if (it == end()) throw OutOfBounds();
    return it->second;
  }
  auto at (const Key &key) const -> const Value & {
    return const_cast<HashMap *>(this)->at(key);
  }

  /**
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  auto operator[] (const Key &key) -> Value & {
    return insert({ key, Value() }).first->second;
  }

  /// behave like at() throw index_out_of_bound if such key does not exist.
  auto operator[] (const Key &key) const -> const Value & { return at(key); }

  /// return a iterator to the beginning
  auto begin () -> iterator { return { pivot_.next_, this }; }
  auto cbegin () const -> const_iterator { return { pivot_.next_, this }; }

  /// return a iterator to the end
  auto end () -> iterator { return { &pivot_, this }; }
  auto cend () const -> const_iterator { return { &pivot_, this }; }

  /// checks whether the container is empty
  auto empty () const -> bool {
    return size_ == 0;
  }
  /// returns the number of elements.
  auto size () const -> size_t {
    return size_;
  }

  /// clears the contents
  auto clear () -> void {
    destroy_();
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert successfully, or false.
   */
  auto insert (const value_type &value) -> Pair<iterator, bool> {
    auto &[ k, _ ] = value;
    auto hash = hash_(k);
    if (capacity_ > 0) {
      int ix = hash & mask[capacity_];
      if (store_[ix].next() != nullptr) {
        Node *node = store_[ix].next()->find(k);
        if (node != nullptr) return { { &node->iteratorList, this }, false };
      }
    }
    growIfNeeded_();
    int ix = hash & mask[capacity_];
    Node *node = new Node(value, hash);
    node->hashList.insertBefore(&store_[ix]);
    node->iteratorList.insertBefore(&pivot_);
    ++size_;
    return { { &node->iteratorList, this }, true };
  }

  /**
   * erase the element at pos.
   * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
   */
  auto erase (iterator pos) -> void {
    if (pos == end() || pos.home_ != this) throw Exception("invalid state");
    pos.node_->self->hashList.remove();
    pos.node_->self->iteratorList.remove();
    delete pos.node_->self;
    pos.node_ = &pivot_;
    --size_;
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   */
  auto count (const Key &key) const -> size_t {
    return find(key) == cend() ? 0 : 1;
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  auto find (const Key &key) -> iterator {
    if (empty()) return end();
    auto ix = hash_(key) & mask[capacity_];
    if (store_[ix].next() == nullptr) return end();
    Node *node = store_[ix].next()->find(key);
    if (node == nullptr) return end();
    return { &node->iteratorList, this };
  }
  auto find (const Key &key) const -> const_iterator {
    return const_cast<HashMap *>(this)->find(key);
  }

 private:
  struct ListNode {
    ListNode *prev_ = this;
    ListNode *next_ = this;
    auto next () -> Node * { return next_->self; }
    auto prev () -> Node * { return prev_->self; }
    Node *self = nullptr;
    ListNode () = default;
    ListNode (Node *node) : self(node) {}

    auto insertBefore (ListNode *pivot) -> void {
      prev_ = pivot->prev_;
      next_ = pivot;
      pivot->prev_ = prev_->next_ = this;
    }
    auto remove () -> void {
      prev_->next_ = next_;
      next_->prev_ = prev_;
    }
    auto init () -> void {
      prev_ = next_ = this;
    }
  };

  struct Node {
    value_type value;
    unsigned hash;
    ListNode iteratorList = this, hashList = this;
    Node () = default;
    Node (const Node &node) : value(node.value), hash(node.hash) {}
    Node (const value_type &value, unsigned hash) : value(value), hash(hash) {}
    auto find (const Key &key) -> Node * {
      if (Equal()(key, value.first)) return this;
      if (hashList.next() == nullptr) return nullptr;
      return hashList.next()->find(key);
    }
  };
  ListNode pivot_;
  ListNode *store_ = nullptr;
  int size_ = 0;
  int capacity_ = 0;
  constexpr static int kThreshold_ = 2;
  Hash hash0_;
  auto hash_ (const Key &key) const -> unsigned {
    return rehash(hash0_(key));
  }
  auto growIfNeeded_ () -> void {
    auto capacityNeeded = static_cast<unsigned long long>((size_ + 1) * kThreshold_);
    if (capacityNeeded > pow2[capacity_]) grow_();
  }
  auto grow_ () -> void {
    if (capacity_ == 0) {
      capacity_ = 2;
      store_ = new ListNode[4];
      return;
    }
    int newCapacity = capacity_ + 1;
    auto prospective = new ListNode[pow2[newCapacity]];
    auto node = &pivot_;
    for (int i = 0; i < size_; ++i) {
      node = node->next_;
      int ix = node->self->hash & mask[newCapacity];
      node->self->hashList.insertBefore(&prospective[ix]);
    }
    capacity_ = newCapacity;
    delete[] store_;
    store_ = prospective;
  }

  auto destroy_ () -> void {
    ListNode *node = pivot_.next_;
    for (int i = 0; i < size_; ++i) {
      ListNode *next = node->next_;
      delete node->self;
      node = next;
    }
    capacity_ = 0;
    size_ = 0;
    delete[] store_;
    store_ = nullptr;
    pivot_.init();
  }
};

} // namespace ticket

#endif // TICKET_LIB_HASHMAP_H_
