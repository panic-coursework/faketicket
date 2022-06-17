#include "train.h"

#include "parser.h"
#include "rollback.h"
#include "utility.h"

namespace ticket {

file::Index<Train::Id, Train> Train::ixId
  {&Train::trainId, "trains.train-id.ix"};
file::BpTree<size_t, int> Train::ixStop {"trains.stop.ix"};

file::Index<Ride, RideSeats> RideSeats::ixRide
  {&RideSeats::ride, "ride-seats.ride.ix"};

// TODO(perf): inline these methods
auto Train::indexOfStop (const std::string &name) const
  -> Optional<int> {
  for (int i = 0; i < stops.length; ++i) {
    // TODO(perf): eliminate this string copy
    if (stops[i].str() == name) return i;
  }
  return unit;
}
auto Train::totalPrice (int ixFrom, int ixTo) const -> int {
  TICKET_ASSERT(ixFrom < ixTo);
  int price = 0;
  for (int i = ixFrom; i < ixTo; ++i) {
    price += edges[i].price;
  }
  return price;
}
auto Train::getRide (Date date) const
  -> Optional<RideSeats> {
  return RideSeats::ixRide.findOne({id(), date});
}
auto Train::getRide (Date date, int ixDeparture) const
  -> Optional<RideSeats> {
  return getRide(
    date - edges[ixDeparture].departure.daysOverflow()
  );
}

auto Ride::operator< (const Ride &rhs) const -> bool {
  // TODO(perf): is this optimization performed by
  // the compiler?
  if (Less<>().ne(date, rhs.date)) return date < rhs.date;
  return train < rhs.train;
}

auto RideSeatsBase::ticketsAvailable (int ixFrom, int ixTo)
  const -> int {
  TICKET_ASSERT(ixFrom > ixTo);
  int min = seatsRemaining[ixFrom];
  for (int i = ixFrom; i < ixTo; ++i) {
    min = std::min(min, seatsRemaining[i]);
  }
  return min;
}
auto RideSeatsBase::rangeAdd (int dx, int ixFrom, int ixTo)
  -> void {
  TICKET_ASSERT(ixFrom > ixTo);
  for (int i = ixFrom; i < ixTo; ++i) {
    seatsRemaining[i] += dx;
  }
}

auto command::run (const command::AddTrain &cmd)
  -> Result<Response, Exception> {
  // TODO
}
auto command::run (const command::DeleteTrain &cmd)
  -> Result<Response, Exception> {
  // TODO
}
auto command::run (const command::ReleaseTrain &cmd)
  -> Result<Response, Exception> {
  // TODO
  Optional<Train> tr = Train::ixId.findOne(cmd.id);
  if (!tr) {
    return Exception("");
  }
  RideSeats rd;
  rd.ride.train = tr->id();
}
auto command::run (const command::QueryTrain &cmd)
  -> Result<Response, Exception> {
  // TODO
}
auto command::run (const command::QueryTicket &cmd)
  -> Result<Response, Exception> {
  // TODO
}
auto command::run (const command::QueryTransfer &cmd)
  -> Result<Response, Exception> {
  // TODO
}

auto rollback::run (const rollback::AddTrain &log)
  -> Result<Unit, Exception> {
  // TODO
}
auto rollback::run (const rollback::DeleteTrain &log)
  -> Result<Unit, Exception> {
  // TODO
}
auto rollback::run (const rollback::ReleaseTrain &log)
  -> Result<Unit, Exception> {
  // TODO
}

} // namespace ticket
