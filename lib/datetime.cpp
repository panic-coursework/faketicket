#include "datetime.h"

#include "utility.h"

// to be optimized: inline these methods

namespace ticket {

namespace {
auto getNumber (const char *str) -> int {
  return (str[0] - '0') * 10 + str[1] - '0';
}
auto getNumbers (const char *str) -> Pair<int, int> {
  return { getNumber(str), getNumber(&str[3]) };
}
auto setNumber (char *str, int i) -> void {
  str[0] = i / 10 + '0';
  str[1] = i % 10 + '0';
}
constexpr int kSzFormat = 6;
auto setNumbers (char *str, int a, int b, char sep) -> void {
  setNumber(str, a); // 0, 1
  str[2] = sep;
  setNumber(&str[3], b); // 3, 4
  str[5] = '\0';
}

constexpr int daysInYear[] = { 0, 30, 61 };
constexpr int kMonthBase = 6;
auto daysFromMd (int month, int date) -> int {
  return daysInYear[month - kMonthBase] + date - 1;
}
auto mdFromDays (int days) -> Pair<int, int> {
  // simplifying the process since there is only 3 months.
  static_assert(sizeof(daysInYear) / sizeof(daysInYear[0]) == 3);
  if (days >= daysInYear[1]) {
    if (days >= daysInYear[2]) {
      return { kMonthBase + 2, days - daysInYear[2] + 1 };
    }
    return { kMonthBase + 1, days - daysInYear[1] + 1 };
  }
  return { kMonthBase + 0, days - daysInYear[0] + 1 };
}

constexpr int kMinutesInHour = 60;
constexpr int kHoursInDay = 24;
auto minutesFromHm (int hour, int minutes) -> int {
  return hour * kMinutesInHour + minutes;
}
auto dhmFromMinutes (int minutes) -> Triple<int, int, int> {
  int hours = minutes / kMinutesInHour;
  return {
    hours / kHoursInDay,
    hours % kHoursInDay,
    minutes % kMinutesInHour
  };
}
} // namespace

Date::Date (const char *str) {
  auto [ month, date ] = getNumbers(str);
  days_ = daysFromMd(month, date);
}
Date::Date (int month, int date) {
  days_ = daysFromMd(month, date);
}

auto Date::month () const -> int {
  return mdFromDays(days_).first;
}
auto Date::date () const -> int {
  return mdFromDays(days_).second;
}
Date::operator std::string () const {
  char buf[kSzFormat];
  auto [ month, date ] = mdFromDays(days_);
  setNumbers(buf, month, date, '-');
  return buf;
}

auto Date::operator+ (int dt) const -> Date {
  return Date(days_ + dt);
}
auto Date::operator- (int dt) const -> Date {
  return Date(days_ - dt);
}
auto Date::operator- (Date rhs) const -> int {
  return days_ - rhs.days_;
}

auto Date::inRange (Date begin, Date end) const -> bool {
  return begin.days_ <= days_ && days_ <= end.days_;
}

auto Duration::minutes () const -> int {
  return minutes_;
}
auto Duration::operator+ (Duration dt) const -> Duration {
  return Duration(minutes_ + dt.minutes_);
}
auto Duration::operator- (Duration dt) const -> Duration {
  return Duration(minutes_ - dt.minutes_);
}
auto Duration::operator- () const -> Duration {
  return Duration(-minutes_);
}

Instant::Instant (int hours, int minutes) {
  minutes_ = minutesFromHm(hours, minutes);
}
Instant::Instant (const char *str) {
  auto [ hours, minutes ] = getNumbers(str);
  minutes_ = minutesFromHm(hours, minutes);
}

auto Instant::daysOverflow () const -> int {
  return dhmFromMinutes(minutes_).first;
}
auto Instant::hour () const -> int {
  return dhmFromMinutes(minutes_).second;
}
auto Instant::minute () const -> int {
  return dhmFromMinutes(minutes_).third;
}

auto Instant::withoutOverflow () const -> Instant {
  return Instant(minutes_ % (kMinutesInHour * kHoursInDay));
}

Instant::operator std::string () const {
  char buf[kSzFormat];
  auto [ _, hours, minutes ] = dhmFromMinutes(minutes_);
  setNumbers(buf, hours, minutes, ':');
  return buf;
}

auto Instant::operator+ (Duration dt) const -> Instant {
  return Instant(minutes_ + dt.minutes());
}
auto Instant::operator- (Duration dt) const -> Instant {
  return Instant(minutes_ - dt.minutes());
}
auto Instant::operator- (Instant rhs) const -> Duration {
  return Duration(minutes_ - rhs.minutes_);
}

auto formatDateTime (Date date, Instant instant)
  -> std::string {
  return std::string(date + instant.daysOverflow()) + " " +
    std::string(instant);
}

} // namespace ticket
