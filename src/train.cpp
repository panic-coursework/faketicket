#include "train.h"

#include "datetime.h"
#include "exception.h"
#include "hashmap.h"
#include "map.h"
#include "parser.h"
#include "run.h"
#include "rollback.h"
#include "utility.h"
#include "vector.h"
#include <sys/_types/_key_t.h>

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
    if(i + 2 < cmd.stations.size()) ins = ins + cmd.stopoverTimes[i];
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
  if (tr->released) return Exception("already released");
  tr->released = true;
  tr->update();

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
  auto train = Train::ixId.findOne(cmd.id);
  if( ! train ) return Exception("No such train");
  if (!cmd.date.inRange(train->begin, train->end)) {
    return Exception("No such ride");
  }

  auto ride = train->getRide( cmd.date);
  if( ! ride ){
    RideSeats nw;
    nw.ride = { train->id(), cmd.date };
    for(int i = 0; i < train->edges.size(); ++ i)
      nw.seatsRemaining.push( train->seats);

    return nw;
  }

  return *ride;
}
auto command::run (const command::QueryTicket &cmd)
  -> Result<Response, Exception> {
  Vector<Range> vct;
  auto v_from = Train::ixStop.findMany( std::hash<std::string>()(cmd.from) );
  auto v_to = Train::ixStop.findMany( std::hash<std::string>()(cmd.to) );

  for(auto ele: v_to)
    v_from.push_back(ele);

  sort( v_from.begin(), v_from.end() );
  v_from.push_back(-1);// for bound
  for(int i = 1; i + 1 < v_from.size(); ++ i)
    if( v_from[i] == v_from[i - 1] && v_from[i] != v_from[i + 1] ){
      Train train;
      train = Train::get(v_from[i]);
      auto ixFrom = train.indexOfStop(cmd.from);
      auto ixTo = train.indexOfStop(cmd.to);

      if( !ixFrom || !ixTo || *ixFrom > *ixTo ) continue;
      auto rd = train.getRide( cmd.date, *ixFrom);
      if( ! rd ) continue;

      long long totPrice = train.totalPrice(*ixFrom, *ixTo);
      auto seats = rd->ticketsAvailable(*ixFrom, *ixTo);

      vct.push_back( ticket::Range( *rd, *ixFrom, *ixTo,
        totPrice, train.edges[*ixTo - 1].arrival - train.edges[*ixFrom].departure, seats, train.trainId ) );
    }

  sort( vct.begin(), vct.end(), Cmp(
    [&cmd] (const Range &r1, const Range &r2) {
      if( cmd.sort == command::kTime){
        if( r1.time != r2.time) return  r1.time < r2.time;
        return r1.trainId < r2.trainId;
      }
      if( r1.totalPrice != r2.totalPrice) return  r1.totalPrice < r2.totalPrice;
      return r1.trainId < r2.trainId;
    }
  )
  );
  return vct;
}

struct KeySection{
  int ixFrom, ixTo;
  // from_station_index; To_station_index
  // one is cmd.from/cmd.to; the other is mid_station
  int train_num_in_vector;
  // the train's index in trains_f/trains_t
  Instant departure, arrival;
  // departure.dateOverflow() should be 0
  Date begin, end;
  // for vFrom: available date for ixFrom
  // for vTo: should all be cmd.date
};

bool cmp_f(const KeySection & r1, const KeySection & r2){
  return r1.arrival < r2.arrival;
}

bool cmp_t(const KeySection & r1, const KeySection & r2){
  return r1.departure < r2.departure;
}

auto command::run (const command::QueryTransfer &cmd)
  -> Result<Response, Exception> {
  using TrainId = int;

  auto v_f = Train::ixStop.findMany( std::hash<std::string>()(cmd.from) );
  auto v_t = Train::ixStop.findMany( std::hash<std::string>()(cmd.to) );

  Vector<Train> trains_f, trains_t;
  int mid_st_no = 0;

  Map<size_t, int> mp;// station::Id.hash() -> mid_st_no;
  Vector<KeySection> vFrom, vTo;

  // TODO: get Vector<KeySection> vFrom, vTo;

  sort(vFrom.begin(), vFrom.end(), cmp_f);
  sort(vTo.begin(), vTo.end(), cmp_t);

  // how to set the compare function
  Map< >mp;

  for(auto &ele: vTo){
  }
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

void Range::output()const{
  const Train &tr = Train::get(rd.ride.train);
  std::cout <<
    tr.trainId << ' ' <<
    tr.stops[ixFrom] << ' '<<
    formatDateTime(rd.ride.date, tr.edges[ixFrom].departure) << ' '<<
    "-> " <<
    tr.stops[ixTo] << ' ' <<
    formatDateTime(rd.ride.date, tr.edges[ixTo - 1].arrival) << ' '<<
    tr.totalPrice(ixFrom, ixTo) << ' ';

  std::cout << rd.ticketsAvailable(ixFrom, ixTo) << std::endl;
}

} // namespace ticket
