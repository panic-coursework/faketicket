#include "rollback.h"

#include "parser.h"

namespace ticket {

file::File<> logEntries {"rollback-log"};

auto command::dispatch (const command::Rollback &cmd) -> void {
  // TODO
}

} // namespace ticket
