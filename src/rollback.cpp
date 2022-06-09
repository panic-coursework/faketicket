#include "rollback.h"

#include "parser.h"

namespace ticket {

static int currentTime;
auto setTimestamp (int timestamp) -> void {
  currentTime = timestamp;
}

auto rollback::log (
  const rollback::LogEntry::Content &content) -> void {
  rollback::LogEntry entry;
  entry.timestamp = currentTime;
  entry.content = content;
  entry.save();
}

auto command::run (const command::Rollback &cmd)
  -> Result<Response, Exception> {
  // TODO
}

} // namespace ticket
