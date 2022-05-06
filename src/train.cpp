#include "train.h"

#include "parser.h"
#include "rollback.h"

namespace ticket {

file::Index<Train::Id, Train> Train::ixId
  {&Train::trainId, "trains.train-id.ix"};
file::BpTree<size_t, int> Train::ixStop {"trains.stop.ix"};

file::Index<Ride, RideSeats> RideSeats::ixRide
  {&RideSeats::ride, "ride-seats.ride.ix"};

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
