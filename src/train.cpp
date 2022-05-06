#include "train.h"

#include "parser.h"
#include "rollback.h"

namespace ticket {

file::File<> trains {"trains"};
file::Index<Train::Id, Train, decltype(trains)>
  ixTrainsId {&Train::trainId, "trains.train-id.ix", trains};
file::BpTree<size_t, int> ixTrainsStop {"trains.stop.ix"};

file::File<> rideSeats {"ride-seats"};
file::Index<Ride, RideSeats, decltype(rideSeats)>
  ixRideSeatsRide {
    &RideSeats::ride,
    "ride-seats.ride.ix",
    rideSeats
  };

auto command::dispatch (const command::AddTrain &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::DeleteTrain &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::ReleaseTrain &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::QueryTrain &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::QueryTicket &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::QueryTransfer &cmd) -> void {
  // TODO
}

auto rollback::dispatch (const rollback::AddTrain &log) -> void {
  // TODO
}
auto rollback::dispatch (const rollback::DeleteTrain &log) -> void {
  // TODO
}
auto rollback::dispatch (const rollback::ReleaseTrain &log) -> void {
  // TODO
}

} // namespace ticket
