#include "order.h"

#include "parser.h"
#include "rollback.h"

namespace ticket {

file::Index<User::Id, Order> Order::ixUserId
  {&Order::user, "orders.user.ix"};

file::Index<Ride, PendingOrder> PendingOrder::ixRide {
  &PendingOrder::ride,
  "pending-orders.ride.ix"
};

auto command::dispatch (const command::BuyTicket &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::QueryOrder &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::RefundTicket &cmd) -> void {
  // TODO
}

auto rollback::dispatch (const rollback::BuyTicket &log) -> void {
  // TODO
}
auto rollback::dispatch (const rollback::RefundTicket &log) -> void {
  // TODO
}

} // namespace ticket
