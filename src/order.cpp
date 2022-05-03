#include "order.h"

#include "parser.h"
#include "rollback.h"

namespace ticket {

file::File<> orders {"orders"};
file::Index<User::Id, Order, decltype(orders)>
  ixOrdersUserId {&Order::user, "orders.user.ix", orders};

file::File<> pendingOrders {"pending-orders"};
file::Index<Ride, PendingOrder, decltype(pendingOrders)>
  ixPendingOrdersRide {
    &PendingOrder::ride,
    "pending-orders.ride.ix",
    pendingOrders
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
