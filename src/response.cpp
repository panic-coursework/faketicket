#include "response.h"
#include "datetime.h"
#include "exception.h"
#include "train.h"

#include <cstddef>
#include <iostream>

namespace ticket::response {

auto cout (const Unit & /* unused */) -> void {
  if constexpr (isInteractive) {
    std::cout << "success!\n";
  } else {
    std::cout << "0\n";
  }
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
      << order.price << ' '
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
    formatDateTime( rd.ride.date, train.edges[i].arrival )
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
auto cout (const Sol & sol) -> void{
  if(sol.empty()){
    std::cout << "0\n";
    return ;
  }
  sol.output();
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
    jsOrder["from"] = JS_STR(cache.from.str());
    jsOrder["to"] = JS_STR(cache.to.str());
    jsOrder["departure"] = JS_DATE(date, cache.timeDeparture);
    jsOrder["arrival"] = JS_DATE(date, cache.timeArrival);
    jsOrder["price"] = JS_NUM(order.price);
    jsOrder["seats"] = JS_NUM(order.seats);
    jsOrder["subTotal"] = JS_NUM(order.getSubTotal());
    jsOrder["status"] = JS_STR(Order::statusString(order.status));
  }
  return arr;
}

auto toJsObject (Napi::Env env, const RideSeats &rd)
  -> Napi::Object {
  auto res = JS_OBJ();
  Train train = Train::get(rd.ride.train);
  res["trainId"] = JS_STR(train.trainId.str());
  char type[2];
  type[0] = train.type;
  type[1] = '\0';
  res["type"] = JS_STR(type);
  auto edges = JS_ARR(train.edges.length);
  for (int i = 0; i < train.edges.length; ++i) {
    auto jsEdge = JS_OBJ();
    jsEdge["departure"] =
      JS_DATE(rd.ride.date, train.edges[i].departure);
    jsEdge["arrival"] =
      JS_DATE(rd.ride.date, train.edges[i].arrival);
    jsEdge["price"] = JS_NUM(train.edges[i].price);
    jsEdge["seatsRemaining"] = JS_NUM(rd.seatsRemaining[i]);
    edges[i] = jsEdge;
  }
  res["edges"] = edges;
  auto stops = JS_ARR(train.stops.length);
  for (int i = 0; i < train.stops.length; ++i) {
    stops[i] = JS_STR(train.stops[i].str());
  }
  res["stops"] = stops;
  return res;
}

inline auto toJsObject (Napi::Env env, const Range &range)
  -> Napi::Object {
  auto res = JS_OBJ();
  Train tr = Train::get(range.rd.ride.train);
  res["trainId"] = JS_STR(tr.trainId.str());
  res["from"] = JS_STR(tr.stops[range.ixFrom].str());
  res["to"] = JS_STR(tr.stops[range.ixTo].str());
  res["timeDeparture"] = JS_DATE(
    range.rd.ride.date, tr.edges[range.ixFrom].departure);
  res["timeArrival"] = JS_DATE(
    range.rd.ride.date, tr.edges[range.ixTo - 1].arrival);
  res["price"] =
    tr.totalPrice(range.ixFrom, range.ixTo);
  res["ticketsAvailable"] =
    range.rd.ticketsAvailable(range.ixFrom, range.ixTo);
  return res;
}

auto toJsObject (Napi::Env env, const Vector<Range> &ranges)
  -> Napi::Object {
  auto res = JS_ARR(0);
  for (int i = 0; i < ranges.size(); ++i) {
    res[i] = toJsObject(env, ranges[i]);
  }
  return res;
}

auto toJsObject (Napi::Env env, const Sol &sol)
  -> Napi::Object {
  auto res = JS_OBJ();
  if (sol.empty()) {
    res["success"] = Napi::Boolean::New(env, false);
    return res;
  }
  res["success"] = Napi::Boolean::New(env, true);

  Range tmp;
  Train train = Train::get(sol.from_mid.trainPos);
  tmp.rd = *train.getRide(sol.date, sol.from_mid.ixKey);
  tmp.ixFrom = sol.from_mid.ixKey;
  tmp.ixTo = sol.from_mid.ixMid;
  tmp.totalPrice = sol.from_mid.totalPrice;
  tmp.time = sol.from_mid.Arrival - sol.from_mid.Departure;
  tmp.trainId = sol.from_mid.trainId;

  res["first"] = toJsObject(env, tmp);

  train = Train::get(sol.mid_to.trainPos);
  tmp.rd = *train.getRide(
    sol.date + sol.mid_to.Departure.daysOverflow(),
    sol.mid_to.ixMid
  );
  tmp.ixFrom = sol.mid_to.ixMid;
  tmp.ixTo = sol.mid_to.ixKey;
  tmp.totalPrice = sol.mid_to.totalPrice;
  tmp.time = sol.mid_to.Arrival - sol.mid_to.Departure;
  tmp.trainId = sol.mid_to.trainId;

  res["second"] = toJsObject(env, tmp);

  return res;
}

#undef JS_OBJ
#undef JS_STR
#undef JS_NUM
#undef JS_ARR
#undef JS_DATE

#endif // BUILD_NODEJS

} // namespace ticket::response

