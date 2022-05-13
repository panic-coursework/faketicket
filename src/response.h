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
  Train,
  Vector<Train>,
  BuyTicketResponse,
  Vector<Order>
  // the exit command does not need a response object.
>;

namespace response {

auto cout (const Unit & /* unused */) -> void;
auto cout (const User &user) -> void;
auto cout (const Train &train) -> void;
auto cout (const Vector<Train> &trains) -> void;
auto cout (const BuyTicketResponse &ticket) -> void;
auto cout (const Vector<Order> &orders) -> void;

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
