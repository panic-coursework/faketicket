#include "rollback.h"

#include "parser.h"
#include "run.h"

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
  rollback::LogEntry::file.setMeta({ entry.id() });
}

auto command::run (const command::Rollback &cmd)
  -> Result<Response, Exception> {
  // TODO
}

} // namespace ticket
