// This file implements the clean and exit commands.
#include "parser.h"

#include <iostream>

#include "order.h"
#include "rollback.h"
#include "train.h"
#include "user.h"

namespace ticket {

auto command::dispatch (const command::Clean & /* unused */)
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
auto command::dispatch (const command::Exit & /* unused */)
  -> Result<Response, Exception> {
  std::cout << "bye\n";
  exit(1);
}

} // namespace ticket

