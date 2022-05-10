#ifndef TICKET_ORDER_H_
#define TICKET_ORDER_H_

#include "file/file.h"
#include "file/index.h"
#include "train.h"
#include "user.h"
#include "variant.h"

namespace ticket {

struct OrderBase {
  using Id = int;
  enum Status { kSuccess, kPending, kRefunded };

  User::Id user;
  Ride ride;
  int ixFrom, ixTo;
  int seats;
  Status status;

  /// gets the corresponding train object.
  auto getTrain () -> Train;

  static constexpr const char *filename = "orders";
};
struct Order : public file::Managed<OrderBase> {
  static file::Index<User::Id, Order> ixUserId;
};


struct PendingOrderBase {
  Ride ride;
  int ixFrom, ixTo;
  int seats;
  Order::Id order;

  /// checks if the order is satisfiable.
  auto satisfiable () -> bool;
  /// gets the corresponding order object.
  auto getOrder () -> Order;

  static constexpr const char *filename = "pending-orders";
};
struct PendingOrder : public file::Managed<PendingOrderBase> {
  static file::Index<Ride, PendingOrder> ixRide;
};

/**
 * @brief Utility class to represent the result of a buy
 * ticket request that a pending order has been created.
 *
 * See BuyTicketResponse below for usage.
 */
struct BuyTicketEnqueued {};
/**
 * @brief Utility class to represent the result of a buy
 * ticket request that the order has been processed.
 *
 * See BuyTicketResponse below for usage.
 */
struct BuyTicketSuccess {
  int price;
};
using BuyTicketResponse = Variant<
  BuyTicketSuccess,
  BuyTicketEnqueued
>;

} // namespace ticket

#endif // TICKET_ORDER_H_
