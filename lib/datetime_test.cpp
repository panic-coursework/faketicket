#include "datetime.h"

#include <cassert>

#include "utility.h"

using ticket::Date, ticket::Duration, ticket::Instant, ticket::Less;

Less<> cmp;

auto testDate () -> void {
  Date d0;
  Date d1(8, 17);
  assert(d1.month() == 8);
  assert(d1.date() == 17);
  assert(d1.inRange(d1, d1));
  Date d2 = ++d1;
  assert(d2 == Date("08-18"));
  assert(cmp.equals(d1, Date("08-18")));
  assert(d1-- == d2);
  assert(static_cast<std::string>(d2) == "08-18");
  assert(d1 < d2);
  assert(d2 - d1 == 1);
  assert(static_cast<std::string>(d2 - 18) == "07-31");
  assert(d2.inRange(Date("08-17"), d2 + 1));
  assert(!d2.inRange(d2 - 10, d2 - 5));
  assert(std::string(Date("06-30") + 1) == "07-01");
  assert(std::string(Date("07-30") + 1) == "07-31");
  assert(std::string(Date("07-31") + 1) == "08-01");
}

auto testDuration () -> void {
  Duration d0;
  Duration d1(130);
  assert(d1.minutes() == 130);
  assert(cmp.equals(Duration(130), d1));
  assert(cmp.equals(Duration(140), d1 + Duration(10)));
  assert(cmp.equals(Duration(120), d1 - Duration(10)));
  assert(cmp.equals(Duration(-10), -Duration(10)));
  assert(Duration(-100).minutes() == -100);
}

auto testInstant () -> void {
  Instant i0;
  Instant i1(2, 10);
  assert(cmp.equals(i1, Instant("02:10")));
  assert(std::string(i1) == "02:10");
  assert(i1.hour() == 2);
  assert(i1.minute() == 10);
  assert(i1.daysOverflow() == 0);
  Duration day(1440);
  assert(cmp.ne(i1, i1 + day));
  assert((i1 + day).hour() == 2);
  assert((i1 + day).minute() == 10);
  assert((i1 + day + day).daysOverflow() == 2);
  assert(cmp.equals(i1 + day + day - day, i1 + day));
  assert(cmp.equals(i1 + day - i1, day));
}

auto main () -> int {
  testDate();
  testDuration();
  testInstant();
  return 0;
}
