#include "train.h"

#include "datetime.h"
#include "exception.h"
#include "hashmap.h"
#include "map.h"
#include "parser.h"
#include "priority-queue.h"
#include "response.h"
#include "run.h"
#include "rollback.h"
#include "utility.h"
#include "vector.h"

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
  if (train != rhs.train) return train < rhs.train;
  return date < rhs.date;
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
  // if (train.trainId.str() == "RQBHVSMsFXrFa")
  //   std::cerr << train.trainId << std::endl;
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
  // if (tr->trainId.str() == "RQBHVSMsFXrFa")
  //   std::cerr << tr->trainId << std::endl;
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
    // if (rd.id() == 81)
    //   std::cerr << rd.id() << std::endl;
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
  // auto ride = train->getRide( cmd.date);
  // if (ride) std::cerr << ride->id() << ' ' << ride->ride.train << ' ' << train->id() << std::endl;
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
        // TODO(perf): copy
        return r1.trainId.str() < r2.trainId.str();
      }
      if( r1.totalPrice != r2.totalPrice)
        return  r1.totalPrice < r2.totalPrice;
      return r1.trainId.str() < r2.trainId.str();
    }
  )
  );
  return vct;
}


struct SectionCmp {
  using Pair = std::pair<Section, Section *>;
  command::SortType type;
  auto operator() (const Pair &lhs, const Pair &rhs) const
    -> bool {
    //  cmd.sort == kTime: smaller section.Departure, section.totalPrice, section.trainID
    //  cmd.sort == kCost: smaller section.totalPrice, section.Departure, section.trainID
    if (type == command::kCost) {
      if (lhs.first.totalPrice != rhs.first.totalPrice) {
        return lhs.first.totalPrice > rhs.first.totalPrice;
      }
    }
    if (lhs.first.Arrival != rhs.first.Arrival) {
      return lhs.first.Arrival > rhs.first.Arrival;
    }
    if (type == command::kTime) {
      if (lhs.first.totalPrice != rhs.first.totalPrice) {
        return lhs.first.totalPrice > rhs.first.totalPrice;
      }
    }
    return lhs.first.trainId.str() > rhs.first.trainId.str();
  }
};

command::SortType Sol::sort;
auto command::run (const command::QueryTransfer &cmd)
  -> Result<Response, Exception> {
  Sol::sort = cmd.sort;
  ////////////////////////////////////////////////////////////////
  // generate: Vector< Vector<Section> > Vf, Vt;
  int _no_st = 0;
  HashMap<size_t, int > no_st;
  Vector< Vector<Section> > Vf, Vt;
  Vf.push_back({});
  Vt.push_back({});

  auto vTrainNum_From =
    Train::ixStop.findMany( std::hash<std::string>()(cmd.from) );
  auto vTrainNum_To =
    Train::ixStop.findMany( std::hash<std::string>()(cmd.to) );

  for(auto & trainPos : vTrainNum_From){
    Train train = Train::get(trainPos);
    Section it;
    it.trainId = train.trainId;
    it.trainPos = train.id();
    it.ixKey = *train.indexOfStop(cmd.from);
    if (it.ixKey == train.stops.length - 1) continue;
    it.Departure =
      train.edges[ it.ixKey ].departure.withoutOverflow();
    // TODO(perf)
    if( ! train.getRide( cmd.date, it.ixKey) ) continue;

    //get st_num

    long long add_price = 0;
    for(int j = it.ixKey + 1; j < train.stops.size(); ++ j){
      add_price += train.edges[j - 1].price;

      int &st_num = no_st[ std::hash<std::string>()(train.stops[j])];
      if( ! st_num ) {
        st_num = ++ _no_st;
        Vf.push_back({});
        Vt.push_back({});
      }
      it.ixMid = j;
      it.Arrival = train.edges[j - 1].arrival
        - (train.edges[it.ixKey].departure
        - it.Departure);
      it.totalPrice = add_price;
      Vf[st_num].push_back(it);
    }
  }

  for(auto trainPos : vTrainNum_To){
    Train train = Train::get(trainPos);
    Section it;
    it.trainId = train.trainId;
    it.trainPos = train.id();
    it.ixKey = *train.indexOfStop(cmd.to);
    it.res = train.end - train.begin;
    if (it.ixKey == 0) continue;
    it.Arrival = train.edges[it.ixKey - 1].arrival
      + Duration( (train.begin - cmd.date) * 24 * 60) ;
    // TO BE CHECKED

    long long add_price = 0;
    for(int j = it.ixKey - 1; j >= 0; --j){
      int &st_num = no_st[ std::hash<std::string>()(train.stops[j])];
      // TODO(perf): continue
      if( ! st_num ) {
        st_num = ++ _no_st;
        Vf.push_back({});
        Vt.push_back({});
      }

      it.ixMid = j;
      it.Departure = it.Arrival
        +(train.edges[j].departure
        - train.edges[ it.ixKey - 1 ].arrival);
      add_price += train.edges[j].price;
      it.totalPrice = add_price;

      //Section validity check
      for(; it.res && it.Departure.daysOverflow() < 0;){
        // TODO(perf)
        -- it.res;
        it.Departure =  it.Departure + Duration(24 * 60);
        it.Arrival = it.Arrival + Duration(24 * 60);
      }
      // ;// std::cerr << "xxx " << it.trainId << it.res << std::string(it.Departure) << std::endl;

      if( it.Departure.daysOverflow() < 0) continue;
      Vt[st_num].push_back(it);
    }
  }
  ////////////////////////////////////////////////////////////////
  // get ANS;
  Sol ANS(cmd.date);
  ;// std::cerr << "--------" << std::endl;
  for(int i = 1; i <= _no_st; ++ i){
    if( Vf[i].empty() || Vt[i].empty()) continue;
    sort(Vf[i].begin(), Vf[i].end(), Cmp([] (const Section &s1, const Section &s2) {
      return s1.Arrival < s2.Arrival;
    }));
    sort(Vt[i].begin(), Vt[i].end(), Cmp([] (const Section &s1, const Section &s2) {
      return s1.Departure.withoutOverflow() < s2.Departure.withoutOverflow();
    }));
    ;// std::cerr << "--- f" << std::endl;
    for (const auto &x : Vf[i]) x.output();
    ;// std::cerr << "--- t" << std::endl;
    for (const auto &x : Vt[i]) x.output();

    SectionCmp cmp {cmd.sort};
    PriorityQueue<std::pair<Section, Section *>, SectionCmp>
      queue(cmp);
    // first: section value when add
    // second: pointer to the real section
    for( ; ! queue.empty(); ) queue.pop();
    for(auto &sec: Vt[i])
      queue.push(std::make_pair( sec, &sec));

    int mover = 0;
    Section * _mid_to = nullptr;
    for(const auto fromTrain : Vf[i]){
      for(int mover = 0; mover < Vt[i].size(); ++mover){
        ;// std::cerr << formatDateTime(cmd.date, Vt[i][mover].Departure) << std::endl;
        ;// std::cerr << formatDateTime(cmd.date, fromTrain.Arrival) << std::endl;
        while (Vt[i][mover].Departure < fromTrain.Arrival) {
          if( Vt[i][mover].res ) {
            ;// std::cerr << "moving " << Vt[i][mover].trainId << std::endl;
            Vt[i][mover].move_to_tomorrow();
            // TODO(perf)
            queue.push({ Vt[i][mover], &Vt[i][mover] });
          } else {
            Vt[i][mover].deleted = true;
            break;
          }
        }
      }

      Section * ans = nullptr;
      for(;! queue.empty() ;){
        auto hd = queue.top();
        hd.first.output();
        // queue.pop();

        // cannot transfer to the same train
        bool sameTrain =
          fromTrain.trainPos == hd.first.trainPos;
        if (sameTrain) {
          queue.pop();
          continue;
        }
        if( hd.second->deleted || (*hd.second) != hd.first) {
          queue.pop();
          continue;
        }
        ans = hd.second;
        break;
      }
      if (ans == nullptr) continue;
      Sol nw( cmd.date, fromTrain, *ans);
      ;// std::cerr << (ans->Arrival - fromTrain.Departure).minutes() << std::endl;
      ;// std::cerr << fromTrain.totalPrice << ' ' << ans->totalPrice << std::endl;
      fromTrain.output();
      ans->output();
      if( ANS.empty() || nw < ANS ) {
        ANS = nw;
        ;// std::cerr << "SET" << std::endl;
      }
    }
  }
  ////////////////////////////////////////////////////////////////
  return ANS;

}

auto rollback::run (const rollback::AddTrain &log)
  -> Result<Unit, Exception> {
  auto train = Train::get(log.id);
  // if (train.trainId.str() == "RQBHVSMsFXrFa")
  //   std::cerr << train.trainId << std::endl;
  Train::ixId.remove(train);
  train.destroy();
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

  for (int i = 0; i < train.stops.length; ++i) {
    Train::ixStop.remove(train.stops[i].hash(), train.id());
  }
  for (auto i = train.begin; i <= train.end; ++i) {
    auto ride = RideSeats::ixRide.findOne({ log.id, i });
    // if (ride->id() == 133 || ride->id() == 146)
    //   std::cerr << ride->id() << std::endl;
    RideSeats::ixRide.remove(*ride);
    // auto ride2 = RideSeats::ixRide.findOne({ log.id, i });
    // if (ride2) {
    //   std::cerr << ride2->id() << '!' << std::endl;
    // }
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

  std::cout << rd.ticketsAvailable(ixFrom, ixTo) << '\n';
}

} // namespace ticket
