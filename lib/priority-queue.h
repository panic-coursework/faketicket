#ifndef TICKET_LIB_PRIORITY_QUEUE_H_
#define TICKET_LIB_PRIORITY_QUEUE_H_

#include <cstddef>
#include <functional>

#include "exception.h"

#ifdef DEBUG
#include <iostream>
#define _log(x) std::clog << x << std::endl;
#else
#define _log(x)
#endif

namespace panic {

template <typename T, class Compare = std::less<T>>
class PairingHeap {
 public:
  struct Node {
    T value;
    Node *firstChild = nullptr;
    Node *neighbor = nullptr;

    Node (const T &value) : value(value) {}
    Node (const Node &other) : value(other.value) {
      // _log("PairingHeap::Node::Node(const Node&)");
      if (other.firstChild != nullptr) firstChild = new Node(*other.firstChild);
      if (other.neighbor != nullptr) neighbor = new Node(*other.neighbor);
    }
    auto destroy () -> void {
      if (firstChild != nullptr) {
        firstChild->destroy();
        delete firstChild;
      }
      if (neighbor != nullptr) {
        neighbor->destroy();
        delete neighbor;
      }
    }
  };

  Compare less;
  Node *root = nullptr;

  /// merges two trees and returns the new root.
  auto mergeRoots (Node *a, Node *b) -> Node * {
    if (a == nullptr) return b;
    if (b == nullptr) return a;
    if (less(a->value, b->value)) std::swap(a, b);
    // a >= b here
    // since b is a tree root, it could not have a neighbor
    b->neighbor = a->firstChild;
    a->firstChild = b;
    return a;
  }

  /// merges all the children of the root node and returns the new root.
  auto mergeChildren (Node *firstChild) -> Node * {
    if (firstChild == nullptr) return nullptr;
    Node *secondChild = firstChild->neighbor;
    if (secondChild == nullptr) return firstChild;
    Node *thirdChild = secondChild->neighbor;
    firstChild->neighbor = secondChild->neighbor = nullptr;
    return mergeRoots(mergeRoots(firstChild, secondChild), mergeChildren(thirdChild));
  }

  PairingHeap () = default;
  PairingHeap (const Compare &cmp) : less(cmp) {}
  PairingHeap (const PairingHeap &other) { *this = other; }
  ~PairingHeap () {
    if (root != nullptr) root->destroy();
    delete root;
  }
  auto operator= (const PairingHeap &other) -> PairingHeap & {
    if (this == &other) return *this;
    if (root != nullptr) {
      root->destroy();
      delete root;
      root = nullptr;
    }
    if (other.root != nullptr) root = new Node(*other.root);
    less = other.less;
    return *this;
  }
};

} // namespace panic

namespace ticket {

template <typename T, class Compare = std::less<T>>
class PriorityQueue {
 public:
  PriorityQueue () = default;
  PriorityQueue (const Compare &cmp) : heap_(cmp) {}
  /**
   * get the top of the queue.
   * @return a reference of the top element.
   * throw container_is_empty if empty() returns true;
   */
  auto top () const -> const T & {
    return heap_.root->value;
  }
  /// push new element to the priority queue.
  auto push (const T &value) -> void {
    Node *newNode = new Node(value);
    heap_.root = heap_.mergeRoots(heap_.root, newNode);
    ++size_;
  }
  /**
   * delete the top element.
   * throw container_is_empty if empty() returns true;
   */
  auto pop () -> void {
    Node *firstChild = heap_.root->firstChild;
    delete heap_.root;
    heap_.root = heap_.mergeChildren(firstChild);
    --size_;
  }
  /// return the number of the elements.
  auto size () const -> size_t { return size_; }
  /**
   * check if the container has at least an element.
   * @return true if it is empty, false if it has at least an element.
   */
  auto empty () const -> bool { return size_ == 0; }
  /**
   * merge two priority_queues with at ~~least~~ most O(logn) complexity.
   * clear the other priority_queue.
   */
  auto merge (PriorityQueue &other) -> void {
    size_ += other.size_;
    other.size_ = 0;
    heap_.root = heap_.mergeRoots(heap_.root, other.heap_.root);
    other.heap_.root = nullptr;
  }

 private:
  using Heap = panic::PairingHeap<T, Compare>;
  using Node = typename Heap::Node;
  Heap heap_;
  size_t size_ = 0;
};

} // namespace ticket

#endif // TICKET_LIB_PRIORITY_QUEUE_H_
