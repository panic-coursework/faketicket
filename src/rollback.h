#ifndef TICKET_BACKLOG_H_
#define TICKET_BACKLOG_H_

#include "file/file.h"
#include "optional.h"
#include "order.h"
#include "train.h"
#include "user.h"
#include "variant.h"

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
  int timestamp;
  Variant<
    AddUser,
    ModifyProfile,
    AddTrain,
    ReleaseTrain,
    BuyTicket,
    RefundTicket
  > content;

  static constexpr const char *filename = "rollback-log";
};
using LogEntry = file::Managed<LogEntryBase>;

/**
 * @brief Visitor for the log entries.
 *
 * The implementations are in the corresponding source
 * files, not in rollback.cpp.
 */
auto dispatch (const AddUser &log) -> void;
auto dispatch (const ModifyProfile &log) -> void;
auto dispatch (const AddTrain &log) -> void;
auto dispatch (const ReleaseTrain &log) -> void;
auto dispatch (const BuyTicket &log) -> void;
auto dispatch (const RefundTicket &log) -> void;

} // namespace ticket::rollback

#endif // TICKET_BACKLOG_H_
