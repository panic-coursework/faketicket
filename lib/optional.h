
#ifndef TICKET_LIB_OPTIONAL_H_
#define TICKET_LIB_OPTIONAL_H_

#include "utility.h"
#include "variant.h"

namespace ticket {

/**
 * @brief A resemblence of std::optional.
 *
 * This class represents a state, or nothing at all. This is
 * sometimes better than using null pointers, as it avoids
 * the problem that a reference cannot be null. Internally
 * it is a variant of Unit and T, therefore some may write
 * Optional<T> = T? = T | Unit = T | null or whatever.
 */
template <typename T>
class Optional : Variant<Unit, T> {
 private:
  using VarT = Variant<Unit, T>;
 public:
  Optional () = default;
  /// constructs a empty optional.
  Optional (Unit /* unused */) : VarT(unit) {}
  /// constructs a filled optional.
  Optional (const T &value) : VarT(value) {}
  auto operator= (const T &value) -> Optional & {
    VarT::operator=(value);
    return *this;
  }
  /// true if the optional has value.
  operator bool () const {
    return this->template is<T>();
  }
  /// provides access to the actual object.
  auto operator* () -> T & {
    return *this->template get<T>();
  }
  auto operator* () const -> const T & {
    return *this->template get<T>();
  }
  auto operator-> () -> T * {
    return this->template get<T>();
  }
  auto operator-> () const -> const T * {
    return this->template get<T>();
  }
};

} // namespace ticket

#endif // TICKET_LIB_OPTIONAL_H_
