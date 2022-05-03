// This file includes date and time utilities.
#ifndef TICKET_LIB_DATETIME_H_
#define TICKET_LIB_DATETIME_H_

#include <iostream>

namespace ticket {

/**
 * @brief Class representing a date between 2021-06-01 and
 * 2021-08-31 (inclusive).
 */
class Date {
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
  /**
   * @brief calculates the difference between two Dates.
   * (06-04 - 06-01 == 3)
   */
  auto operator- (Date rhs) const -> int;
  auto operator< (const Date &rhs) const -> bool;
  /// checks if this Date is in the given range (inclusive).
  auto inRange (Date begin, Date end) const -> bool;
 private:
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
class Duration {
 public:
  Duration () = default;
  Duration (int hour, int minute);
  explicit Duration (int minutes);
  /// constructs a Duration from an HH:MM format string.
  explicit Duration (const char *str);
  /// gets the hour part of the duration, may be negative.
  auto hours () const -> int;
  /// gets the minute part of the duration, may be negative.
  auto minutes () const -> int;
  /// gets how many minutes are there in this Duration.
  auto totalMinutes () const -> int;
  auto operator+ (Duration dt) const -> Duration;
  auto operator- (Duration dt) const -> Duration;
  /// negates the Duration.
  auto operator- () const -> Duration;
  auto operator< (const Duration &rhs) const -> bool;
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
class Instant {
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
  auto operator< (const Instant &rhs) const -> bool;
 private:
  int minutes_ = 0;
};

} // namespace ticket

#endif // TICKET_LIB_DATETIME_H_