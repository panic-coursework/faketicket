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
  /// checks if the pending order is satisfiable.
  auto satisfiable () -> bool;

  static constexpr const char *filename = "orders";
};
struct Order : public file::Managed<OrderBase> {
  Order () = default;
  Order (const file::Managed<OrderBase> &order)
    : file::Managed<OrderBase>(order) {}
  static file::Index<User::Id, Order> ixUserId;
  static file::Index<Ride, Order> pendingOrders;
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
