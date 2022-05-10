#include "order.h"

#include "parser.h"
#include "rollback.h"

namespace ticket {

file::Index<User::Id, Order> Order::ixUserId
  {&Order::user, "orders.user.ix"};

file::Index<Ride, Order> Order::pendingOrders {
  &Order::ride,
  "orders-pending.ride.ix"
};

auto command::dispatch (const command::BuyTicket &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::QueryOrder &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::RefundTicket &cmd) -> Result<Response, Exception> {
  // TODO
}

auto rollback::dispatch (const rollback::BuyTicket &log) -> Result<Unit, Exception> {
  // TODO
}
auto rollback::dispatch (const rollback::RefundTicket &log) -> Result<Unit, Exception> {
  // TODO
}

} // namespace ticket
