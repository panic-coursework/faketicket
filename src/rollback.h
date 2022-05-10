#ifndef TICKET_BACKLOG_H_
#define TICKET_BACKLOG_H_

#include "file/file.h"
#include "optional.h"
#include "order.h"
#include "train.h"
#include "user.h"
#include "variant.h"

namespace ticket {
/// sets the current timestamp.
auto setTimestamp (int timestamp) -> void;
} // namespace ticket


namespace ticket::rollback {

struct AddUser {
  int id;
};

struct ModifyProfile {
  int id;
  Optional<User::Password> password;
  Optional<User::Name> name;
  Optional<User::Email> email;
  Optional<User::Privilege> privilege;
};

struct AddTrain {
  int id;
};

struct DeleteTrain {
  int id;
};

struct ReleaseTrain {
  int id;
};

struct BuyTicket {
  int id;
};

struct RefundTicket {
  int id;
  Order::Status status;
};

struct LogEntryBase {
  using Content = Variant<
    AddUser,
    ModifyProfile,
    AddTrain,
    DeleteTrain,
    ReleaseTrain,
    BuyTicket,
    RefundTicket
  >;

  int timestamp;
  Content content;

  static constexpr const char *filename = "rollback-log";
};
using LogEntry = file::Managed<LogEntryBase>;

/// inserts a log entry.
auto log (const LogEntry::Content &content) -> void;

/**
 * @brief Visitor for the log entries.
 *
 * The implementations are in the corresponding source
 * files, not in rollback.cpp.
 */
auto dispatch (const AddUser &log) -> Result<Unit, Exception>;
auto dispatch (const ModifyProfile &log) -> Result<Unit, Exception>;
auto dispatch (const AddTrain &log) -> Result<Unit, Exception>;
auto dispatch (const DeleteTrain &log) -> Result<Unit, Exception>;
auto dispatch (const ReleaseTrain &log) -> Result<Unit, Exception>;
auto dispatch (const BuyTicket &log) -> Result<Unit, Exception>;
auto dispatch (const RefundTicket &log) -> Result<Unit, Exception>;

} // namespace ticket::rollback

#endif // TICKET_BACKLOG_H_
