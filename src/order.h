#ifndef TICKET_ORDER_H_
#define TICKET_ORDER_H_

#include "file/file.h"
#include "file/index.h"
#include "train.h"
#include "user.h"

namespace ticket {

struct Order : public file::ManagedObject<Order> {
  using Id = int;
  enum Status { kSuccess, kPending, kRefunded };

  User::Id user;
  Ride ride;
  int ixFrom, ixTo;
  int seats;
  Status status;

  /// gets the corresponding train object.
  auto getTrain () -> Train;
};

extern file::File<> orders;
extern file::Index<User::Id, Order, decltype(orders)>
  ixOrdersUserId;

struct PendingOrder : public file::ManagedObject<PendingOrder> {
  Ride ride;
  int ixFrom, ixTo;
  int seats;
  Order::Id order;

  /// checks if the order is satisfiable.
  auto satisfiable () -> bool;
  /// gets the corresponding order object.
  auto getOrder () -> Order;
};

extern file::File<> pendingOrders;
extern file::Index<Ride, PendingOrder, decltype(pendingOrders)>
  ixPendingOrdersRide;

} // namespace ticket

#endif // TICKET_ORDER_H_
