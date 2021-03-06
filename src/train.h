#ifndef TICKET_TRAIN_H_
#define TICKET_TRAIN_H_

#include "datetime.h"
#include "exception.h"
#include "file/array.h"
#include "file/bptree.h"
#include "file/file.h"
#include "file/index.h"
#include "file/varchar.h"
#include "optional.h"
#include "parser.h"
#include <string>

namespace ticket {

namespace Station {
using Id = file::Varchar<30>;
} // namespace Station

struct RideSeats;

struct TrainBase {
  using Id = file::Varchar<20>;
  using Type = char;
  struct Edge {
    int price;
    Instant departure;
    Instant arrival;
  };

  Id trainId;
  file::Array<Station::Id, 100> stops;
  file::Array<Edge, 99> edges;
  int seats;
  Date begin, end;
  Type type;
  bool released = false;
  bool deleted = false;

  // save() when first called
  // update() when members changed

  static constexpr const char *filename = "trains";
};
struct Train : public file::Managed<TrainBase> {
  Train () = default;
  Train (const file::Managed<TrainBase> &train)
    : file::Managed<TrainBase>(train) {}
  // every record with its identifier
  static file::Index<Train::Id, Train> ixId; // maintain it
  // deleted = 0
  static file::BpTree<size_t, int> ixStop; // maintain it
  // released = 1

  /// finds the index of the station of the given name.
  auto indexOfStop (const std::string &name) const
    -> Optional<int>;
  /// calculates the total price of a trip.
  auto totalPrice (int ixFrom, int ixTo) const -> int;

  /**
   * @brief gets the remaining seats object on a given date.
   * @param date the departure date of the entire train
   *             (i.e. not the departure date of a stop).
   */
  auto getRide (Date date) const -> Optional<RideSeats>;
  /**
   * @brief gets the remaining seats object on a given date
   *        at a given stop.
   * @param date the departure date of a stop.
   * @param ixDeparture the index of the departing stop.
   */
  auto getRide (Date date, int ixDeparture) const
    -> Optional<RideSeats>;
};


struct Ride {
  /// the numerical id of the train.
  int train;
  Date date;

  auto operator< (const Ride &rhs) const -> bool;
};

struct Range;
struct RideSeatsBase {
  Ride ride;
  file::Array<int, 99> seatsRemaining;// maintain it

  /**
   * @brief calculates how many tickets are still available.
   * @param ixFrom index of the departing stop
   * @param ixTo index of the arriving stop
   */
  auto ticketsAvailable (int ixFrom, int ixTo) const -> int;
  /// adds dx to seatsRemaining[ixFrom, ixTo] inclusive.
  auto rangeAdd (int dx, int ixFrom, int ixTo) -> void;

  static constexpr const char *filename = "ride-seats";
};
struct RideSeats : public file::Managed<RideSeatsBase> {
  RideSeats () = default;
  RideSeats (const file::Managed<RideSeatsBase> &rideSeats)
    : file::Managed<RideSeatsBase>(rideSeats) {}
  static file::Index<Ride, RideSeats> ixRide;// maintain it
};

struct Range{
  using Id = file::Varchar<20>;

  static command::SortType sort;

  RideSeats rd;
  int ixFrom, ixTo;
  long long totalPrice;
  Duration time;
  int seats;
  Id trainId;

  Range(): ixFrom(-1), ixTo(-1){};
  Range(RideSeats _rd, int _ixFrom, int _ixTo
  , long long _totalPrice, Duration _time,int _seats, Id _id):
   rd(_rd), ixFrom(_ixFrom), ixTo(_ixTo),
   totalPrice(_totalPrice), time(_time), seats(_seats),trainId(_id){};

  void output()const;
};

struct Section{
  using Id = file::Varchar<20>;
  Id trainId;
  int trainPos = -1;
  int ixKey, ixMid;
  Instant Departure, Arrival;// all based on cmd.date
  long long totalPrice;
  // Departure.daysOverflow() = 0
  int res;// Just for midSt->To
  //init: res = train.end - train.begin

  bool deleted = false;

  bool operator !=(const Section& other) const{
    return Departure != other.Departure
      || Arrival != other.Arrival
      || totalPrice != other.totalPrice
      || deleted != other.deleted;
  }
  bool move_to_tomorrow(){
    if( res <= 0 ) return false;
    -- res;
    Departure = Departure + Duration(24 * 60);
    Arrival = Arrival + Duration(24 * 60);
    return true;
  }

  void output()const{
    ;// std::cerr << trainId << std::endl;
    auto t = Train::get(trainPos);
    ;// std::cerr << t.stops[ixKey] << t.stops[ixMid] << std::endl;
    ;// std::cerr << (Departure.daysOverflow()) << ' ' << (Arrival.daysOverflow()) << std::endl;
    ;// std::cerr << std::string(Departure) << ' ' << std::string(Arrival) << std::endl;
    // TO DO
  }
};

struct Sol{
  static command::SortType sort;
  Section from_mid, mid_to;
  Date date;
  Sol(const Date &dt): date(dt){};
  Sol(const Date &dt, const Section &from, const Section &to):
    date(dt),from_mid(from), mid_to(to) {}
  long long price()const{
    return from_mid.totalPrice + mid_to.totalPrice;
  }
  Duration time()const{
    return mid_to.Arrival - from_mid.Departure;
  }
  bool empty()const{ return from_mid.trainPos == -1;}
  bool operator<(const Sol &rhs)const{
    if(sort == command::kTime){
      if( time() != rhs.time() ) return time() < rhs.time();
      if( price() != rhs.price() ) return price() < rhs.price();
    }
    else{
      if( price() != rhs.price() ) return price() < rhs.price();
      if( time() != rhs.time() ) return time() < rhs.time();
    }
    if( from_mid.trainId != rhs.from_mid.trainId)
      return from_mid.trainId.str() < rhs.from_mid.trainId.str();
    return mid_to.trainId.str() < rhs.mid_to.trainId.str();
  }
  void output()const{
    Range tmp;
    Train train = Train::get(from_mid.trainPos);
    tmp.rd = *train.getRide(date, from_mid.ixKey);
    tmp.ixFrom = from_mid.ixKey;
    tmp.ixTo = from_mid.ixMid;
    tmp.totalPrice = from_mid.totalPrice;
    tmp.time = from_mid.Arrival - from_mid.Departure;
    tmp.trainId = from_mid.trainId;

    tmp.output();

    train = Train::get(mid_to.trainPos);
    ;// std::cerr << std::string(train.begin) << std::string(train.end) << std::endl;
    ;// std::cerr << std::string(date + mid_to.Departure.daysOverflow()) << " " << mid_to.ixMid << train.trainId << std::endl;
    tmp.rd = *train.getRide(date + mid_to.Departure.daysOverflow(), mid_to.ixMid);
    tmp.ixFrom = mid_to.ixMid;
    tmp.ixTo = mid_to.ixKey;
    tmp.totalPrice = mid_to.totalPrice;
    tmp.time = mid_to.Arrival - mid_to.Departure;
    tmp.trainId = mid_to.trainId;

    tmp.output();

  }
};

} // namespace ticket

#endif // TICKET_TRAIN_H_
