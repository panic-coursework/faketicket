/**
 * This file defines exception classes used throughout the
 * project. Throwing exceptions is not encouraged, since it
 * has a poor stack unwinding performance.
 */
#ifndef TICKET_LIB_EXCEPTION_H_
#define TICKET_LIB_EXCEPTION_H_

namespace ticket {

/// The base exception class.
class Exception {
 public:
  Exception () = default;
  Exception (const char *what) : what_(what) {}
  virtual ~Exception () = default;
  /// returns a human-readable description of the exception.
  virtual auto what () -> const char * {
    return what_;
  }
 private:
  const char * const what_ = "unknown exception";
};

class IoException : public Exception {
 public:
  IoException () : Exception("IO exception") {}
  IoException (const char *what) : Exception(what) {}
};

class OutOfBounds : public Exception {
 public:
  OutOfBounds () : Exception("out of bounds") {}
  OutOfBounds (const char *what) : Exception(what) {}
};

class Overflow : public OutOfBounds {
 public:
  Overflow () : OutOfBounds("overflow") {}
  Overflow (const char *what) : OutOfBounds(what) {}
};

class Underflow : public OutOfBounds {
 public:
  Underflow () : OutOfBounds("underflow") {}
  Underflow (const char *what) : OutOfBounds(what) {}
};

class NotFound : public Exception {
 public:
  NotFound () : Exception("underflow") {}
  NotFound (const char *what) : Exception(what) {}
};

} // namespace ticket

#endif // TICKET_LIB_EXCEPTION_H_
