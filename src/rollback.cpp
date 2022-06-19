#include "rollback.h"

#include "exception.h"
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
  if (cmd.timestamp > currentTime) {
    return Exception("rollback target is in the future");
  }

  User::clearSessions();

  int lastId = rollback::LogEntry::file.getMeta().id;
  while (lastId >= 0) {
    auto entry = rollback::LogEntry::get(lastId);
    if (entry.timestamp <= cmd.timestamp) break;
    --lastId;
    entry.content.visit([] (const auto &cmd) {
      auto res = rollback::run(cmd);
      if (auto err = res.error()) {
        // There must be something going unwildly wrong.
        throw *err;
      }
    });
    entry.destroy();
  }
  rollback::LogEntry::file.setMeta({ lastId });
  return unit;
}

} // namespace ticket
