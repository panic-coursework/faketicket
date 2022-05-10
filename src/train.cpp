#include "train.h"

#include "parser.h"
#include "rollback.h"

namespace ticket {

file::Index<Train::Id, Train> Train::ixId
  {&Train::trainId, "trains.train-id.ix"};
file::BpTree<size_t, int> Train::ixStop {"trains.stop.ix"};

file::Index<Ride, RideSeats> RideSeats::ixRide
  {&RideSeats::ride, "ride-seats.ride.ix"};

auto command::dispatch (const command::AddTrain &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::DeleteTrain &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::ReleaseTrain &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::QueryTrain &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::QueryTicket &cmd) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::QueryTransfer &cmd) -> Result<Response, Exception> {
  // TODO
}

auto rollback::dispatch (const rollback::AddTrain &log) -> Result<Unit, Exception> {
  // TODO
}
auto rollback::dispatch (const rollback::DeleteTrain &log) -> Result<Unit, Exception> {
  // TODO
}
auto rollback::dispatch (const rollback::ReleaseTrain &log) -> Result<Unit, Exception> {
  // TODO
}

} // namespace ticket
