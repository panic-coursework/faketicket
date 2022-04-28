#ifndef TICKET_LIB_RESULT_H_
#define TICKET_LIB_RESULT_H_

#include "utility.h"
#include "variant.h"

namespace ticket {

/**
 * Result<Res, Err> = Res | Err
 *
 * This class provides a wrapper around variant to make
 * error handling a little easier. Recommended usage:
 *
 * auto foo = doSomethingThatMightFail(args);
 * if (auto err = foo.error()) {
 *   // handles error, or rethrow:
 *   return *err;
 * }
 * std::cout << foo.result() << std::endl;
 *
 * Therefore, result() returns a reference, while error()
 * returns a pointer. This design is subject to change.
 */
template <typename ResultType, typename ErrorType>
class Result : public Variant<ResultType, ErrorType> {
 public:
  Result () = delete;
  template <typename T>
  Result (const T &value) : Variant<ResultType, ErrorType>(value) {}
  auto result () -> ResultType & {
    return *this->template get<ResultType>();
  }
  auto result () const -> const ResultType & {
    return *this->template get<ResultType>();
  }
  auto error () -> ErrorType * {
    return this->template get<ErrorType>();
  }
  auto error () const -> const ErrorType * {
    return this->template get<ErrorType>();
  }

  /// returns true if the result is in its successful state.
  auto success () const -> bool {
    return this->index() == 0;
  }
};

} // namespace ticket

#endif // TICKET_LIB_RESULT_H_
