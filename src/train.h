#ifndef TICKET_TRAIN_H_
#define TICKET_TRAIN_H_

#include "datetime.h"
#include "exception.h"
#include "file/array.h"
#include "file/bptree.h"
#include "file/file.h"
#include "file/index.h"
#include "file/varchar.h"
#include "result.h"

namespace ticket {

namespace Station {
using Id = file::Varchar<30>;
} // namespace Station

struct RideSeats;

struct TrainBase {
  using Id = file::Varchar<20>;
  using Type = char;
  struct Stop {
    Station::Id name;
  };
  struct Edge {
    int price;
    Instant departure;
    Instant arrival;
  };

  Id trainId;
  file::Array<Stop, 100> stops;
  file::Array<Edge, 99> edges;
  int seats;
  Date begin, end;
  Type type;
  bool released;

  /// finds the index of the station of the given name.
  auto indexOfStop (const std::string &name) -> Result<int, NotFound>;
  /// calculates the total price of a trip.
  auto totalPrice (int ixDeparture, int ixArrival) -> int;

  /**
   * @brief gets the remaining seats object on a given date.
   * @param date the departure date of the entire train
   *             (i.e. not the departure date of a stop).
   */
  auto getRide (Date date) -> RideSeats;
  /**
   * @brief gets the remaining seats object on a given date
   *        at a given stop.
   * @param date the departure date of a stop.
   * @param ixDeparture the index of the departing stop.
   */
  auto getRide (Date date, int ixDeparture) -> RideSeats;

  /**
   * @brief checks if the train has a ride departing from
   *        the first station on the given date.
   * @param date the departure date of the first station.
   */
  auto runsOnDate (Date date) -> bool;
  /**
   * @brief checks if the train has a ride departing from
   *        the given station on the given date.
   * @param date the departure date of the given station.
   * @param ixDeparture the index of the departing stop.
   */
  auto runsOnDate (Date date, int ixDeparture) -> bool;

  static constexpr const char *filename = "trains";
};
struct Train : public file::Managed<TrainBase> {
  static file::Index<Train::Id, Train> ixId;
  static file::BpTree<size_t, int> ixStop;
};


struct Ride {
  /// the numerical id of the train.
  int train;
  Date date;

  auto operator< (const Ride &rhs) const -> bool;
};

struct RideSeatsBase {
  Ride ride;
  file::Array<int, 99> seatsRemaining;

  /**
   * @brief calculates how many tickets are still available.
   * @param ixFrom index of the departing stop
   * @param ixTo index of the arriving stop
   */
  auto ticketsAvailable (int ixFrom, int ixTo) -> int;

  static constexpr const char *filename = "ride-seats";
};
struct RideSeats : public file::Managed<RideSeatsBase> {
  static file::Index<Ride, RideSeats> ixRide;
};

} // namespace ticket

#endif // TICKET_TRAIN_H_
