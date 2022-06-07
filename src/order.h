#ifndef TICKET_ORDER_H_
#define TICKET_ORDER_H_

#include "file/file.h"
#include "file/index.h"
#include "train.h"
#include "user.h"
#include "variant.h"

namespace ticket {

struct OrderCache {
  Train::Id trainId;
  Station::Id from, to;
  Instant timeDeparture, timeArrival;
};

struct OrderBase {
  using Id = int;
  enum Status { kSuccess, kPending, kRefunded };
  /// gets the string representation of the status.
  inline static auto statusString (Status status)
    -> const char * {
    switch (status) {
      case kSuccess: return "success";
      case kPending: return "pending";
      case kRefunded: return "refunded";
    }
  }

  User::Id user;
  Ride ride;
  int ixFrom, ixTo;
  int seats;
  int price;
  Status status;
  OrderCache cache;

  /// gets the corresponding train object.
  auto getTrain () -> Train;

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
