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

#include "internal/rehash.inc"

/**
 * @brief An unordered hash-based map.
 *
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
    store_ = new ListNode[internal::pow2[capacity_]];
    const ListNode *node = &other.pivot_;
    for (int i = 0; i < size_; ++i) {
      node = node->next_;
      Node *newNode = new Node(*(node->self));
      int ix = newNode->hash & internal::mask[capacity_];
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
      int ix = hash & internal::mask[capacity_];
      if (store_[ix].next() != nullptr) {
        Node *node = store_[ix].next()->find(k);
        if (node != nullptr) return { { &node->iteratorList, this }, false };
      }
    }
    growIfNeeded_();
    int ix = hash & internal::mask[capacity_];
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
    auto ix = hash_(key) & internal::mask[capacity_];
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
    return internal::rehash(hash0_(key));
  }
  auto growIfNeeded_ () -> void {
    auto capacityNeeded = static_cast<unsigned long long>((size_ + 1) * kThreshold_);
    if (capacityNeeded > internal::pow2[capacity_]) grow_();
  }
  auto grow_ () -> void {
    if (capacity_ == 0) {
      capacity_ = 2;
      store_ = new ListNode[4];
      return;
    }
    int newCapacity = capacity_ + 1;
    auto prospective = new ListNode[internal::pow2[newCapacity]];
    auto node = &pivot_;
    for (int i = 0; i < size_; ++i) {
      node = node->next_;
      int ix = node->self->hash & internal::mask[newCapacity];
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
