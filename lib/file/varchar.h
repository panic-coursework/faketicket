#ifndef TICKET_LIB_FILE_VARCHAR_H_
#define TICKET_LIB_FILE_VARCHAR_H_

#include <cstring>
#include <iostream>

#include "exception.h"

namespace ticket::file {

/**
 * @brief A wrapper for const char * with utility functions
 * and type conversions.
 *
 * the trailing zero is not counted in maxLength.
 *
 * its default ordering is hash order. this is for a
 * maximum performance. you need to write a comparator
 * if you want dictionary order.
 */
template <int maxLength>
struct Varchar {
 public:
  static constexpr int kMaxLength = maxLength;
  Varchar () { content[0] = '\0'; }
  Varchar (const std::string &s) {
    if (s.length() > maxLength) {
      throw Overflow("Varchar length overflow");
    }
    strncpy(content, s.c_str(), maxLength + 1);
  }
  Varchar (const char *cstr) : Varchar(std::string(cstr)) {
    if (strlen(cstr) > maxLength) {
      throw Overflow("Varchar length overflow");
    }
    strncpy(content, cstr, maxLength + 1);
  }

  template<int A>
  Varchar (const Varchar<A> &that) { *this = that; }
  operator std::string () const {
    return std::string(content);
  }
  [[nodiscard]] auto str () const -> std::string {
    return std::string(*this);
  }

  auto length () const -> int {
    return strlen(content);
  }

  template <int A>
  auto operator= (const Varchar<A> &that) -> Varchar & {
    if (that.length() > maxLength) {
      throw Overflow("Varchar length overflow");
    }
    strcpy(content, that.content);
    hash_ = that.hash_;
    return *this;
  }

  template <int A>
  auto operator< (const Varchar<A> &that) const -> bool {
    return hash() < that.hash();
  }
  template <int A>
  auto operator== (const Varchar<A> &that) const -> bool {
    return hash() == that.hash();
  }
  template <int A>
  auto operator!= (const Varchar<A> &that) const -> bool {
    return hash() != that.hash();
  }

  auto hash () const -> size_t {
    if (hash_ != 0) return hash_;
    return hash_ = std::hash<std::string_view>()(content);
  }

 private:
  template <int A>
  friend class Varchar;
  char content[maxLength + 1];
  mutable size_t hash_ = 0;
};

} // namespace ticket::file

#endif // TICKET_LIB_FILE_VARCHAR_H_
