#ifndef TICKET_LIB_FILE_ARRAY_H_
#define TICKET_LIB_FILE_ARRAY_H_

#include <cstring>
#include <functional>

#include "exception.h"
#include "utility.h"

namespace ticket::file {

/**
 * An on-stack array with utility functions and bound checks.
 * The value type needs to be trivial.
 */
template <typename T, size_t maxLength, typename Cmp = Less<T>>
struct Array {
 private:
  auto boundsCheck_ (size_t index) -> void {
    if (index >= length) throw OutOfBounds("Array: overflow or underflow");
  }
  Cmp cmp_;
 public:
  size_t length = 0;
  T content[maxLength];
  /// finds the index of element in the array.
  auto indexOf (const T &element) -> size_t {
    for (size_t i = 0; i < length; ++i) {
      if (cmp_.equals(element, content[i])) return i;
    }
    throw NotFound("Array::indexOf: element not found");
  }
  /// checks if the elements is included in the array.
  auto includes (const T &element) -> bool {
    for (size_t i = 0; i < length; ++i) {
      if (cmp_.equals(element, content[i])) return true;
    }
    return false;
  }
  /**
   * moves the elements after offset backwards, and inserts
   * the element at the offset.
   */
  auto insert (const T &element, size_t offset) -> void {
    if (offset != length) boundsCheck_(offset);
    if (length == maxLength) {
      throw Overflow("Array::insert: overflow");
    }
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

  /// removes the element, and moves forward the elements after it.
  auto remove (const T &element) -> void {
    removeAt(indexOf(element));
  }
  /**
   * removes the element at offset, and moves forward the
   * elements after it.
   */
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
  /// clears the array.
  auto clear () -> void { length = 0; }

  /// copies a portion of another array to this.
  auto copyFrom (
    const Array &other,
    size_t ixFrom,
    size_t ixTo,
    size_t count
  ) -> void {
    if (this == &other) {
      memmove(
        &content[ixTo],
        &content[ixFrom],
        count * sizeof(content[0])
      );
    } else {
      memcpy(
        &content[ixTo],
        &other.content[ixFrom],
        count * sizeof(content[0])
      );
    }
  }

  auto operator[] (size_t index) -> T & {
    boundsCheck_(index);
    return content[index];
  }
  auto operator[] (size_t index) const -> const T & {
    boundsCheck_(index);
    return content[index];
  }

  /// pops the last element.
  auto pop () -> T {
    if (length == 0) throw Underflow("Array::pop: underflow");
    return content[--length];
  }
  /// pops the first element.
  auto shift () -> T {
    if (length == 0) throw Underflow("Array::pop: underflow");
    T result = content[0];
    removeAt(0);
    return result;
  }
  /// pushes after the last element.
  auto push (const T &object) -> T { insert(object, length); }
  /// pushes before the first element.
  auto unshift (const T &object) -> T { insert(object, 0); }

  /// calls the callback for each element in the array.
  auto forEach (const std::function<void (const T &)> &callback) -> T {
    for (size_t i = 0; i < length; ++i) callback(content[i]);
  }
};

} // namespace ticket::file

#endif // TICKET_LIB_FILE_ARRAY_H_
