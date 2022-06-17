// This file includes date and time utilities.
#ifndef TICKET_LIB_DATETIME_H_
#define TICKET_LIB_DATETIME_H_

#include <iostream>

#include "utility.h"

namespace ticket {

/**
 * @brief Class representing a date between 2021-06-01 and
 * 2021-08-31 (inclusive).
 */
class Date : public Comparable<Date> {
 public:
  Date () = default;
  Date (int month, int date);
  /**
   * @brief constructs a Date from a MM-DD format string.
   *
   * it is an undefined behavior if the string is not in
   * MM-DD format, is nullptr, or points to invalid memory.
   */
  explicit Date (const char *str);
  /// gets the month of the Date. (Fri Jun 04 2021 -> 6)
  auto month () const -> int;
  /// gets the date of the Date. (Fri Jun 04 2021 -> 4)
  auto date () const -> int;
  /// gets a MM-DD representation of the Date.
  operator std::string () const;
  /**
   * @brief calculates a date dt days after this Date.
   * (06-04 + 3 == 06-07)
   */
  auto operator+ (int dt) const -> Date;
  /**
   * @brief calculates a date dt days before this Date.
   * (06-04 - 3 == 06-01)
   */
  auto operator- (int dt) const -> Date;
  auto operator++ () -> Date & {
    ++days_;
    return *this;
  }
  auto operator++ (int) -> Date {
    Date res = *this;
    ++days_;
    return res;
  }
  auto operator-- () -> Date & {
    --days_;
    return *this;
  }
  auto operator-- (int) -> Date {
    Date res = *this;
    --days_;
    return res;
  }
  /**
   * @brief calculates the difference between two Dates.
   * (06-04 - 06-01 == 3)
   */
  auto operator- (Date rhs) const -> int;
  auto cmp (Date rhs) const -> int {
    return *this - rhs;
  }
  /// checks if this Date is in the given range (inclusive).
  auto inRange (Date begin, Date end) const -> bool;
 private:
  explicit Date (int days) : days_(days) {}
  int days_ = 0;
};

/**
 * @brief Class representing a length of timespan.
 *
 * The length may be positive, zero or negative.
 *
 * Not to be confused with Instant, which is a fixed point
 * of time. For example, 02:10 as in “brewing the tea takes
 * 02:10” is a duration, while 02:10 as in “it's 02:10 now,
 * go to sleep right now” is an instant.
 */
class Duration : public Comparable<Duration> {
 public:
  Duration () = default;
  explicit Duration (int minutes) : minutes_(minutes) {}
  /// gets how many minutes are there in this Duration.
  auto minutes () const -> int;
  auto operator+ (Duration dt) const -> Duration;
  auto operator- (Duration dt) const -> Duration;
  /// negates the Duration.
  auto operator- () const -> Duration;
  auto cmp (Duration rhs) const -> int {
    return minutes_ - rhs.minutes_;
  }
 private:
  int minutes_ = 0;
};

/**
 * @brief Class representing a point of time in a day.
 *
 * An Instant may overflow, and this class takes care of
 * that by daysOverflow().
 *
 * Not to be confused with Duration, see notes in Duration.
 */
class Instant : public Comparable<Instant> {
 public:
  Instant () = default;
  Instant (int hour, int minute);
  /// constructs an Instant from an HH:MM format string.
  explicit Instant (const char *str);
  auto daysOverflow () const -> int;
  auto hour () const -> int;
  auto minute () const -> int;
  /// gets an HH:MM representation of the Instant.
  operator std::string () const;
  auto operator+ (Duration dt) const -> Instant;
  auto operator- (Duration dt) const -> Instant;
  auto operator- (Instant rhs) const -> Duration;
  auto cmp (Instant rhs) const -> int {
    return minutes_ - rhs.minutes_;
  }
 private:
  explicit Instant (int minutes) : minutes_(minutes) {}
  int minutes_ = 0;
};

auto formatDateTime (Date date, Instant instant)
  -> std::string;

} // namespace ticket

#endif // TICKET_LIB_DATETIME_H_
