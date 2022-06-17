#include "response.h"
#include "datetime.h"
#include "exception.h"
#include "train.h"

#include <cstddef>
#include <iostream>

namespace ticket::response {

auto cout (const Unit & /* unused */) -> void {
  std::cout << "0\n";
}
auto cout (const User &user) -> void {
  std::cout
    << user.username << ' '
    << user.name << ' '
    << user.email << ' '
    << user.privilege << '\n';
}
auto cout (const BuyTicketResponse &ticket) -> void {
  if (auto succ = ticket.get<BuyTicketSuccess>()) {
    std::cout << succ->price << '\n';
  } else {
    std::cout << "queue\n";
  }
}
auto cout (const Vector<Order> &orders) -> void {
  std::cout << orders.size() << '\n';
  for (const auto &order : orders) {
    const auto &cache = order.cache;
    auto date = order.ride.date;
    std::cout
      << '[' << Order::statusString(order.status) << "] "
      << cache.trainId.str() << ' '
      << cache.from.str() << ' '
      << formatDateTime(date, cache.timeDeparture) << " -> "
      << cache.to.str() << ' '
      << formatDateTime(date, cache.timeArrival) << ' '
      << order.getSubTotal() << ' '
      << order.seats << '\n';
  }
}
/**
  * output format:
  *  <departure[i] formatDateTime>' '<price[i]>' '<seats[i]>'\n' 
  *<stations[i+1]>' '<<arrival[i] formatDateTime>"-> "
*/
auto cout (const RideSeats &rd) -> void{
  Train train = Train::get( rd.ride.train );
  std::cout << train.trainId << ' ' << train.type << '\n';
  
  // from 
  std::cout << train.stops[0] << " xx-xx xx:xx -> ";

  long long tot_price = 0;
  for(int i = 0; i < train.edges.size(); ++ i){
    std :: cout << 
    formatDateTime( rd.ride.date, train.edges[i].departure )
    << ' ' << tot_price << ' ' << rd.seatsRemaining[i] <<'\n'
    << train.stops[i + 1] << ' ' << 
    formatDateTime( rd.ride.date, train.edges[i + 1].arrival )
    << " -> ";

    tot_price += train.edges[i].price;
  }
  //to
  std::cout << "xx-xx xx:xx " << tot_price << " x\n";
}
auto cout (const Vector<Range> & ranges) -> void{
  std::cout << ranges.size() << '\n';
  for(auto &ele: ranges)
    ele.output();
}



#ifdef BUILD_NODEJS

#define JS_OBJ() Napi::Object::New(env)
#define JS_STR(x) Napi::String::New(env, x)
#define JS_NUM(x) Napi::Number::New(env, x)
#define JS_ARR(length) Napi::Array::New(env, length)
#define JS_DATE(date, instant) \
  Napi::Value(env.Global()["Date"]).As<Napi::Function>() \
  .New({ JS_STR(formatDateTime(date, instant)) });

auto toJsObject (Napi::Env env, const Unit & /* unused */)
  -> Napi::Object {
  auto obj = JS_OBJ();
  obj["success"] = Napi::Boolean::New(env, true);
  return obj;
}
auto toJsObject (Napi::Env env, const User &user)
  -> Napi::Object {
  auto obj = JS_OBJ();
  obj["username"] = JS_STR(user.username.str());
  obj["name"] = JS_STR(user.name.str());
  obj["email"] = JS_STR(user.email.str());
  obj["privilege"] = JS_NUM(user.privilege);
  return obj;
}
auto toJsObject (Napi::Env env, const Train &train)
  -> Napi::Object {
  // TODO
}
auto toJsObject (Napi::Env env, const Vector<Train> &trains)
  -> Napi::Object {
  // TODO
}

auto toJsObject (
  Napi::Env env,
  const BuyTicketResponse &ticket
) -> Napi::Object {
  auto obj = JS_OBJ();
  if (auto succ = ticket.get<BuyTicketSuccess>()) {
    obj["status"] = JS_STR("success");
    obj["price"] = JS_NUM(succ->price);
  } else {
    obj["status"] = JS_STR("enqueued");
  }
  return obj;
}

auto toJsObject (Napi::Env env, const Vector<Order> &orders)
  -> Napi::Object {
  auto arr = JS_ARR(0);
  for (int i = 0; i < orders.size(); ++i) {
    auto jsOrder = JS_OBJ();
    arr[i] = jsOrder;
    const auto &order = orders[i];
    const auto &cache = order.cache;
    auto date = order.ride.date;
    jsOrder["id"] = JS_NUM(order.id());
    jsOrder["trainId"] = JS_STR(cache.trainId.str());
    jsOrder["from"] = JS_DATE(date, cache.timeDeparture);
    jsOrder["to"] = JS_DATE(date, cache.timeArrival);
    jsOrder["price"] = JS_NUM(order.price);
    jsOrder["seats"] = JS_NUM(order.seats);
    jsOrder["subTotal"] = JS_NUM(order.getSubTotal());
  }
  return arr;
}

#undef JS_OBJ
#undef JS_STR
#undef JS_NUM
#undef JS_ARR
#undef JS_DATE

#endif // BUILD_NODEJS

} // namespace ticket::response

