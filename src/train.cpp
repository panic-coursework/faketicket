#include "train.h"

#include "datetime.h"
#include "exception.h"
#include "parser.h"
#include "run.h"
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
  if( Train::ixId.findOneId(cmd.id) ) return Exception("Train is already exists");
  Train train;
  train.trainId = cmd.id;
  train.type = cmd.type;
  train.begin = cmd.dates[0];
  train.end = cmd.dates[1];
  train.seats = cmd.seats;

  for(const auto & s : cmd.stations) train.stops.push(s);
  Instant ins = cmd.departure;
  for(int i = 0; i + 1 < cmd.stations.size(); ++ i){
    train.edges.push( {cmd.prices[i], ins, ins + cmd.durations[i]} );
    ins = ins + cmd.durations[i];
    if(i + 1 < cmd.stations.size()) ins = ins + cmd.stopoverTimes[i];
  }

  train.save();
  Train::ixId.insert(train);

  return unit;
}
auto command::run (const command::DeleteTrain &cmd)
  -> Result<Response, Exception> {
  auto tr = Train :: ixId . findOne( cmd.id );
  if( ! tr ) return Exception("No train found");
  if( tr -> released ) return Exception("Failed to delete");
  tr -> deleted = true;
  tr -> update();
  Train::ixId.remove(*tr);

  return unit;
}
auto command::run (const command::ReleaseTrain &cmd)
  -> Result<Response, Exception> {
  auto tr = Train::ixId.findOne(cmd.id);
  if( ! tr ) return Exception("No such train");
  tr->released = true;

  const size_t cnt_dur = tr->edges.length;
  const int _seats = tr->seats;

  for(int j = 0; j < cnt_dur + 1; ++j)
    Train::ixStop.insert( tr->stops[j].hash(), tr->id() );

  for(auto i = tr->begin; i <= tr->end; ++ i){
    RideSeats rd;
    rd.ride.train = tr -> id();
    for(int j = 1; j <= cnt_dur; ++ j)
      rd.seatsRemaining.push(_seats);
    rd.ride.date = i;
    rd.save();
    RideSeats :: ixRide.insert(rd);
  }
  return unit;
}
auto command::run (const command::QueryTrain &cmd)
  -> Result<Response, Exception> {

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
