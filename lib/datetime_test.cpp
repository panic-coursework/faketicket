#include "datetime.h"

#include <cassert>

using ticket::Date, ticket::Duration, ticket::Instant;

auto testDate () -> void {
  Date d0;
  Date d1(8, 17);
  assert(d1.month() == 8);
  assert(d1.date() == 17);
  assert(d1.inRange(d1, d1));
  Date d2 = d1 + 1;
  Date d3("08-18");
  assert(!(d2 < d3 || d3 < d2));
  assert(static_cast<std::string>(d2) == "08-18");
  assert(d1 < d2);
  assert(d2 - d1 == 1);
  assert(static_cast<std::string>(d2 - 18) == "07-31");
  assert(d2.inRange(d2 - 1, d2 + 1));
  assert(!d2.inRange(d2 - 10, d2 - 5));
}

// TODO
auto testDuration () -> void {}

auto testInstant () -> void {}

auto main () -> int {
  testDate();
  testDuration();
  testInstant();
  return 0;
}
