#include "order.h"

#include "algorithm.h"
#include "parser.h"
#include "rollback.h"
#include "run.h"
#include "user.h"

namespace ticket {

file::Index<User::Id, Order> Order::ixUserId
  {&Order::user, "orders.user.ix"};

file::Index<Ride, Order> Order::pendingOrders {
  &Order::ride,
  "orders-pending.ride.ix"
};

auto OrderBase::getSubTotal () const -> long long {
  return static_cast<long long>(price) * seats;
}

auto command::run (const command::BuyTicket &cmd)
  -> Result<Response, Exception> {
  if (!User::isLoggedIn(cmd.currentUser)) {
    return Exception("not logged in");
  }

  auto train = Train::ixId.findOne(cmd.train);
  if (!train || train->deleted) {
    return Exception("no such train");
  }
  if (cmd.seats > train->seats) {
    return Exception("too many seats for this train");
  }

  auto ixFrom = train->indexOfStop(cmd.from);
  auto ixTo = train->indexOfStop(cmd.to);
  if (!ixFrom || !ixTo) return Exception("no such station");
  if (*ixFrom >= *ixTo) {
    return Exception("the train runs in the opposite way");
  }

  auto seatsInfo = train->getRide(cmd.date, *ixFrom);
  if (!seatsInfo) {
    return Exception("no such train on this date");
  }

  Order order;
  order.user = cmd.currentUser;
  order.ride = seatsInfo->ride;
  order.ixFrom = *ixFrom;
  order.ixTo = *ixTo;
  order.price = train->totalPrice(*ixFrom, *ixTo);
  order.seats = cmd.seats;

  auto &cache = order.cache;
  cache.trainId = train->trainId;
  cache.timeArrival = train->edges[*ixTo - 1].arrival;
  cache.timeDeparture = train->edges[*ixFrom].departure;
  cache.from = cmd.from;
  cache.to = cmd.to;

  if (seatsInfo->ticketsAvailable(*ixFrom, *ixTo) < cmd.seats) {
    if (!cmd.queue) return Exception("not enough tickets");
    order.status = Order::kPending;
  } else {
    order.status = Order::kSuccess;
    seatsInfo->rangeAdd(-cmd.seats, *ixFrom, *ixTo);
    seatsInfo->update();
  }

  order.save();
  Order::ixUserId.insert(order);
  if (order.status == Order::kPending) {
    Order::pendingOrders.insert(order);
  }
  rollback::log(rollback::BuyTicket{(int) order.id()});

  if (order.status == Order::kPending) {
    return BuyTicketResponse(BuyTicketEnqueued());
  }
  return BuyTicketResponse(BuyTicketSuccess{
    order.getSubTotal()
  });
}

auto command::run (const command::QueryOrder &cmd)
  -> Result<Response, Exception> {
  if (!User::isLoggedIn(cmd.currentUser)) {
    return Exception("not logged in");
  }

  auto orderIds =
    Order::ixUserId.findManyId(cmd.currentUser);
  sort(orderIds.begin(), orderIds.end(), Greater<>());

  // not orderIds.map(Order::get) here because a wrap object
  // is needed for Variant
  return orderIds.map([] (const auto &x) -> Order {
    return Order::get(x);
  });
}

auto command::run (const command::RefundTicket &cmd)
  -> Result<Response, Exception> {
  if (!User::isLoggedIn(cmd.currentUser)) {
    return Exception("not logged in");
  }

  auto orderIds =
    Order::ixUserId.findManyId(cmd.currentUser);
  sort(orderIds.begin(), orderIds.end(), Greater<>());
  if (cmd.index > orderIds.size()) {
    return Exception("no such order");
  }
  auto order = Order::get(orderIds[cmd.index - 1]);
  if (order.status == Order::kRefunded) {
    return Exception("the order has already been refunded");
  }

  rollback::RefundTicket log;
  log.id = order.id();
  log.status = order.status;
  rollback::log(log);

  // deleting a pending order does not need to check other
  // pending orders.
  if (order.status == Order::kPending) {
    Order::pendingOrders.remove(order);
    order.status = Order::kRefunded;
    order.update();
    return unit;
  }

  auto seats = RideSeats::ixRide.findOne(order.ride);
  seats->rangeAdd(order.seats, order.ixFrom, order.ixTo);

  // ok, let's check for other pending orders.
  auto pending = Order::pendingOrders.findMany(order.ride);
  // TODO(perf): speed up the for loop (RMQ?)
  for (auto &target : pending) {
    auto max =
      seats->ticketsAvailable(target.ixFrom, target.ixTo);
    if (max < target.seats) continue;

    seats->rangeAdd(
      -target.seats,
      target.ixFrom,
      target.ixTo
    );

    target.status = Order::kSuccess;
    target.update();
    Order::pendingOrders.remove(target);

    rollback::log(rollback::FulfillOrder{target.id()});
  }
  seats->update();

  return unit;
}

auto rollback::run (const rollback::BuyTicket &log)
  -> Result<Unit, Exception> {
  auto order = Order::get(log.id);
  if (order.status == Order::kPending) {
    Order::pendingOrders.remove(order);
  }
  Order::ixUserId.remove(order);
  order.destroy();
  return unit;
}

auto rollback::run (const rollback::RefundTicket &log)
  -> Result<Unit, Exception> {
  // we only need to undo the refund operation. Fulfilled
  // orders will be undone in its own function.
  auto order = Order::get(log.id);
  TICKET_ASSERT(order.status == Order::kRefunded);
  order.status = log.status;
  order.update();

  if (order.status == Order::kSuccess) {
    // deref it here as we are sure that it exists.
    auto ride = *RideSeats::ixRide.findOne(order.ride);
    ride.rangeAdd(-order.seats, order.ixFrom, order.ixTo);
    ride.update();
  } else {
    Order::pendingOrders.insert(order);
  }
  return unit;
}

auto rollback::run (const rollback::FulfillOrder &log)
  -> Result<Unit, Exception> {
  auto order = Order::get(log.id);
  TICKET_ASSERT(order.status == Order::kSuccess);
  order.status = Order::kPending;
  order.update();
  Order::pendingOrders.insert(order);

  auto ride = *RideSeats::ixRide.findOne(order.ride);
  ride.rangeAdd(order.seats, order.ixFrom, order.ixTo);
  ride.update();
  return unit;
}

} // namespace ticket
