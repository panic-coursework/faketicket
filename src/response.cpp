#include "response.h"

#include <iostream>

namespace ticket::response {

auto cout (const Unit & /* unused */) -> void {
  std::cout << "0\n";
}
auto cout (const User &user) -> void {
  // TODO
}
auto cout (const Train &train) -> void {
  // TODO
}
auto cout (const Vector<Train> &trains) -> void {
  // TODO
}
auto cout (const BuyTicketResponse &ticket) -> void {
  // TODO
}
auto cout (const Order &order) -> void {
  // TODO
}

#ifdef BUILD_NODEJS

#define JS_OBJ() Napi::Object::New(env)
#define JS_STR(x) Napi::String::New(env, x)
#define JS_NUM(x) Napi::Number::New(env, x)
#define JS_ARR(length) Napi::Array::New(env, length)

auto toJsObject (Napi::Env env, const Unit & /* unused */) -> Napi::Object {
  return Napi::Object::New(env);
}
auto toJsObject (Napi::Env env, const User &user) -> Napi::Object {
  auto obj = JS_OBJ();
  obj["username"] = JS_STR(user.username.str());
  obj["name"] = JS_STR(user.name.str());
  obj["email"] = JS_STR(user.email.str());
  obj["privilege"] = JS_NUM(user.privilege);
  return obj;
}
auto toJsObject (Napi::Env env, const Train &train) -> Napi::Object {
  // TODO
}
auto toJsObject (Napi::Env env, const Vector<Train> &trains) -> Napi::Object {
  // TODO
}
auto toJsObject (Napi::Env env, const BuyTicketResponse &ticket) -> Napi::Object {
  auto obj = JS_OBJ();
  if (ticket.is<BuyTicketEnqueued>()) {
    obj["status"] = JS_STR("enqueued");
  } else {
    obj["status"] = JS_STR("success");
    obj["price"] = JS_NUM(ticket.get<BuyTicketSuccess>()->price);
  }
  return obj;
}
auto toJsObject (Napi::Env env, const Order &order) -> Napi::Object {
  // TODO
}

#undef JS_OBJ
#undef JS_STR
#undef JS_NUM
#undef JS_ARR

#endif // BUILD_NODEJS

} // namespace ticket::response

