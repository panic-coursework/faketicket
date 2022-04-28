#ifndef TICKET_LIB_FILE_SET_H_
#define TICKET_LIB_FILE_SET_H_

#include <cstring>
#include <functional>

#include "algorithm.h"
#include "exception.h"
#include "utility.h"

// FIXME: remove dupe code of Set and Array. does C++ support mixins?
namespace ticket::file {

/// A sorted array with utility functions and bound checks.
template <typename T, size_t maxLength, typename Cmp = Less<T>>
struct Set {
 private:
  auto boundsCheck_ (size_t index) -> void {
    if (index >= length) {
      throw OutOfBounds("Set: overflow or underflow");
    }
  }
  Cmp cmp_;
 public:
  Set () = default;
  size_t length = 0;
  T content[maxLength];
  auto indexOfInsert (const T &element) -> size_t {
    return lowerBound(content, content + length, element) - content;
  }
  /// finds the index of element in the set.
  auto indexOf (const T &element) -> size_t {
    size_t index = indexOfInsert(element);
    if (index >= length || !cmp_.equals(content[index], element)) {
      throw NotFound("Set::indexOf: element not found");
    }
    return index;
  }
  /// checks if the elements is included in the set.
  auto includes (const T &element) -> bool {
    size_t ix = indexOfInsert(element);
    return ix < length && cmp_.equals(content[ix], element);
  }
  /// inserts the element into the set.
  auto insert (const T &element) -> void {
    if (length == maxLength) {
      throw Overflow("Set::insert: overflow");
    }
    size_t offset = indexOfInsert(element);
    if (offset != length) {
      memmove(
        &content[offset + 1],
        &content[offset],
        (length - offset) * sizeof(content[0])
      );
    }
    content[offset] = element;
    ++length;
  }

  /// removes the element from the set.
  auto remove (const T &element) -> void {
    removeAt(indexOf(element));
  }
  /// removes the element at offset.
  auto removeAt (size_t offset) -> void {
    boundsCheck_(offset);
    if (offset != length - 1) {
      memmove(
        &content[offset],
        &content[offset + 1],
        (length - offset - 1) * sizeof(content[0])
      );
    }
    --length;
  }
  /// clears the set.
  auto clear () -> void { length = 0; }

  auto operator[] (size_t index) -> T & {
    boundsCheck_(index);
    return content[index];
  }
  auto operator[] (size_t index) const -> const T & {
    boundsCheck_(index);
    return content[index];
  }

  /// pops the greatest element.
  auto pop () -> T {
    if (length == 0) throw Underflow("Set::pop: underflow");
    return content[--length];
  }
  /// pops the least element.
  auto shift () -> T {
    if (length == 0) throw Underflow("Set::pop: underflow");
    T result = content[0];
    removeAt(0);
    return result;
  }

  /// calls the callback for each element in the array.
  auto forEach (const std::function<void (const T &element)> &callback) -> void {
    for (int i = 0; i < length; ++i) callback(content[i]);
  }
};

} // namespace ticket::file

#endif // TICKET_LIB_FILE_SET_H_
