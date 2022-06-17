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
  static file::Index<Train::Id, Train> ixId; // maintain it
  static file::BpTree<size_t, int> ixStop; // maintain it

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
  static file::Index<Ride, RideSeats> ixRide;
};

struct Range{
  RideSeats *rd;
  int ixFrom, ixTo;

  Range(): rd(nullptr){}
  Range(RideSeats * _rd, int _ixFrom, int _ixTo):
   rd(_rd), ixFrom(_ixFrom), ixTo(_ixTo){};
  
  void output()const;
};

} // namespace ticket

#endif // TICKET_TRAIN_H_
