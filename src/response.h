// TODO: docs
#ifndef TICKET_RESPONSE_H_
#define TICKET_RESPONSE_H_

#ifdef BUILD_NODEJS
#include <napi.h>
#endif // BUILD_NODEJS

#include "order.h"
#include "train.h"
#include "user.h"
#include "utility.h"
#include "variant.h"

namespace ticket {

using Response = Variant<
  Unit,
  User,
  BuyTicketResponse,
  Vector<Order>,
  RideSeats,
  Vector<Range>,
  Sol
  // the exit command does not need a response object.
>;

constexpr static bool isInteractive =
#ifdef TICKET_INTERACTIVE
  true;
#else
  false;
#endif

namespace response {

//  corrections guar
auto cout (const Unit & /* unused */) -> void;
auto cout (const User &user) -> void;
auto cout (const BuyTicketResponse &ticket) -> void;
auto cout (const Vector<Order> &orders) -> void;
auto cout (const RideSeats &rd) -> void;// for "QueryTrain"
auto cout (const Vector<Range> & ranges) -> void;// for "QueryTicket"
auto cout (const Sol & sol) -> void;// for "QueryTransfer"

#ifdef BUILD_NODEJS

auto toJsObject (Napi::Env env, const Unit & /* unused */) -> Napi::Object;
auto toJsObject (Napi::Env env, const User &user) -> Napi::Object;
auto toJsObject (Napi::Env env, const Train &train) -> Napi::Object;
auto toJsObject (Napi::Env env, const Vector<Train> &trains) -> Napi::Object;
auto toJsObject (Napi::Env env, const BuyTicketResponse &ticket) -> Napi::Object;
auto toJsObject (Napi::Env env, const Vector<Order> &orders) -> Napi::Object;

#endif // BUILD_NODEJS

} // namespace response

} // namespace ticket

#endif // TICKET_RESPONSE_H_
