#include "train.h"

#include "datetime.h"
#include "exception.h"
#include "hashmap.h"
#include "map.h"
#include "parser.h"
#include "response.h"
#include "run.h"
#include "rollback.h"
#include "utility.h"
#include "vector.h"
#include <cstddef>
#include <queue>
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
  if( Train::ixId.findOneId(cmd.id) )
    return Exception("Train is already exists");
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
  rollback::log(rollback::AddTrain { train.id() });

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
  rollback::log(rollback::DeleteTrain { tr->id() });

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

  rollback::log(rollback::ReleaseTrain { tr->id() });

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
        totPrice, train.edges[*ixTo - 1].arrival
          - train.edges[*ixFrom].departure, seats, train.trainId ) );
    }

  sort( vct.begin(), vct.end(), Cmp(
    [&cmd] (const Range &r1, const Range &r2) {
      if( cmd.sort == command::kTime){
        if( r1.time != r2.time) return  r1.time < r2.time;
        return r1.trainId < r2.trainId;
      }
      if( r1.totalPrice != r2.totalPrice)
        return  r1.totalPrice < r2.totalPrice;
      return r1.trainId < r2.trainId;
    }
  )
  );
  return vct;
}


bool cmp_ar(const Section &s1, const Section &s2){
  return s1.Arrival.withoutOverflow() < s2.Arrival.withoutOverflow();
}
bool cmp_de(const Section &s1, const Section &s2){
  return s1.Departure.withoutOverflow() < s2.Departure.withoutOverflow();
}

auto command::run (const command::QueryTransfer &cmd)
  -> Result<Response, Exception> {
    Sol::sort = cmd.sort;
  ////////////////////////////////////////////////////////////////
  // generate: Vector< Vector<Section> > Vf, Vt;
  int _no_st = 0;
  Map<size_t, int > no_st;
  Vector< Vector<Section> > Vf, Vt;

  auto vTrainNum_From =
    Train::ixStop.findMany( std::hash<std::string>()(cmd.from) );
  auto vTrainNum_To =
    Train::ixStop.findMany( std::hash<std::string>()(cmd.to) );

  Vector<Train> TrainsFrom, TrainsTo;
  for(auto &train_num: vTrainNum_From)
    TrainsFrom.push_back( Train::get(train_num) );
  for(auto &train_num: vTrainNum_To)
    TrainsTo.push_back( Train::get(train_num) );

  for(int i = 0; i < TrainsFrom.size(); ++ i){
    Train &train = TrainsFrom[i];

    Section it;
    it.trainId = train.trainId;
    it.trainNum = i;
    it.ixKey = train.indexOfStop(cmd.from);
    it.Departure =
      train.edges[ it.ixKey ].arrival.withoutOverflow();
    if( ! train.getRide( cmd.date, it.ixKey) ) continue;

    //get st_num

    long long add_price = 0;
    for(int j = it.ixKey + 1; j < train.stops.size(); ++ j){
      add_price += train.edges[j - 1].price;

      int &st_num = no_st[ std::hash<std::string>()(train.stops[j])];
      if( ! st_num ) st_num = ++ _no_st;
      it.ixMid = j;
      it.Arrival = train.edges[j - 1].arrival
        - (train.edges[it.ixKey].departure
        - it.Departure);
      it.totalPrice = add_price;
      Vf[st_num].push_back(it);
    }
  }

  for(int i = 0; i < TrainsTo.size(); ++ i){
    Train &train = TrainsTo[i];

    Section it;
    it.trainId = train.trainId;
    it.trainNum = i;
    it.ixKey = train.indexOfStop(cmd.to);
    it.Arrival = train.edges[it.ixKey - 1].arrival
      + Duration( (train.begin - cmd.date) * 24 * 60) ;
    // TO BE CHECKED

    long long add_price = 0;
    for(int j = 0; j < it.ixKey; ++ j){
      add_price += train.edges[j].price;
      int &st_num = no_st[ std::hash<std::string>()(train.stops[j])];
      if( ! st_num ) st_num = ++ _no_st;

      it.res = train.end - train.begin;
      it.ixMid = j;
      it.Departure = it.Arrival
        +(train.edges[j].departure
        - train.edges[ it.ixKey ].arrival);
      // TO BE CHECKED: can Duration be negative?
      it.totalPrice = add_price;

      //Section validity check
      for(; it.res && it.Departure.daysOverflow() < 0;){
        -- it.res;
        it.Departure =  it.Departure + Duration(24 * 60);
        it.Arrival = it.Arrival + Duration(24 * 60);
      }

      if( it.Departure.daysOverflow() < 0) continue;
      Vt[st_num].push_back(it);
    }
  }
  ////////////////////////////////////////////////////////////////
  // get ANS;
  Sol ANS(cmd.date);
  for(int i = 1; i <= _no_st; ++ i){
    if( Vf[i].empty() || Vt[i].empty()) continue;
    sort(Vf[i].begin(), Vf[i].end(), cmp_ar);
    sort(Vt[i].begin(), Vt[i].end(), cmp_de);

    std::priority_queue<std::pair<Section, Section *> > queue;
    // first: section value when add
    // second: pointer to the real section
    for( ; ! queue.empty(); ) queue.pop();
    // TO DO: reset priority( smaller Instant prioritized )
    //  cmd.sort == kTime: smaller section.Departure, section.totalPrice, section.trainID
    //  cmd.sort == kCost: smaller section.totalPrice, section.Departure, section.trainID
    for(auto &sec: Vt[i])
      queue.push(std::make_pair( sec, &sec));

    int mover = 0;
    Section * _mid_to = nullptr;
    for(int j = 0; j < Vf[i].size(); ++ j){
      for(; mover + 1 < Vt[i].size()
        && Vt[i][mover].Departure < Vf[i][j].Arrival;){
          if( Vt[i][mover].res ) Vt[i][mover].move_to_tomorrow();
          else Vt[i][mover].deleted = true;
        }

      Section * ans = nullptr;
      for(;! queue.empty() ;){
        auto hd = queue.top();
        queue.pop();

        if( hd.second->deleted || (*hd.second) != hd.first)
          continue;
        ans = hd.second;
        break;
      }
      Sol nw( cmd.date, Vf[i][j], *ans);
      if( ANS.empty() || nw < ANS ) ANS = nw;
    }
  }
  ////////////////////////////////////////////////////////////////
  return ANS;

}

auto rollback::run (const rollback::AddTrain &log)
  -> Result<Unit, Exception> {
  Train::get(log.id).destroy();
  return unit;
}
auto rollback::run (const rollback::DeleteTrain &log)
  -> Result<Unit, Exception> {
  auto train = Train::get(log.id);
  train.deleted = false;
  train.update();
  Train::ixId.insert(train);
  return unit;
}
auto rollback::run (const rollback::ReleaseTrain &log)
  -> Result<Unit, Exception> {
  auto train = Train::get(log.id);
  train.released = false;
  train.update();

  Train::ixStop.remove(train.trainId.hash(), train.id());
  for (auto i = train.begin; i != train.end; ++i) {
    auto ride = RideSeats::ixRide.findOne({ log.id, i });
    ride->destroy();
  }

  return unit;
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
