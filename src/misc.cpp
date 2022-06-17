// This file implements the clean and exit commands.
#include "run.h"

#include <iostream>

#include "order.h"
#include "rollback.h"
#include "train.h"
#include "user.h"

namespace ticket {

auto command::run (const command::Clean & /* unused */)
  -> Result<Response, Exception> {
  Order::truncate();
  Order::ixUserId.truncate();
  Order::pendingOrders.truncate();
  rollback::LogEntry::truncate();
  Train::truncate();
  Train::ixId.truncate();
  Train::ixStop.truncate();
  RideSeats::truncate();
  RideSeats::ixRide.truncate();
  User::truncate();
  User::ixUsername.truncate();
  return unit;
}
auto command::run (const command::Exit & /* unused */)
  -> Result<Response, Exception> {
  std::cout << "bye\n";
  exit(0);
}

} // namespace ticket

