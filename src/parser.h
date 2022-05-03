// This file is autogenerated. Do not modify.
#ifndef TICKET_PARSER_H_
#define TICKET_PARSER_H_

#include <iostream>

#include "datetime.h"
#include "exception.h"
#include "optional.h"
#include "variant.h"
#include "result.h"

/// Classes and parsers for commands.
namespace ticket::command {

enum SortType { kTime, kCost };

struct AddUser {
  Optional<std::string> currentUser;
  std::string username;
  std::string password;
  std::string name;
  std::string email;
  Optional<int> privilege;
};

struct Login {
  std::string username;
  std::string password;
};

struct Logout {
  std::string username;
};

struct QueryProfile {
  std::string currentUser;
  std::string username;
};

struct ModifyProfile {
  std::string currentUser;
  std::string username;
  Optional<std::string> password;
  Optional<std::string> name;
  Optional<std::string> email;
  Optional<int> privilege;
};

struct AddTrain {
  std::string id;
  int stops;
  int seats;
  Vector<std::string> stations;
  Vector<int> prices;
  Instant departure;
  Vector<Duration> durations;
  Vector<Duration> stopoverTimes;
  Vector<Date> dates;
  char type;
};

struct ReleaseTrain {
  std::string id;
};

struct QueryTrain {
  std::string id;
  Date date;
};

struct QueryTicket {
  std::string from;
  std::string to;
  Date date;
  SortType sort = kTime;
};

struct QueryTransfer {
  std::string from;
  std::string to;
  Date date;
  SortType sort = kTime;
};

struct BuyTicket {
  std::string currentUser;
  std::string train;
  Date date;
  int seats;
  std::string from;
  std::string to;
  bool queue = false;
};

struct QueryOrder {
  std::string currentUser;
};

struct RefundTicket {
  std::string currentUser;
  int index = 1;
};

struct Rollback {
  int timestamp;
};

struct Clean {
  
};

struct Exit {
  
};


using Command = Variant<
  AddUser,
  Login,
  Logout,
  QueryProfile,
  ModifyProfile,
  AddTrain,
  ReleaseTrain,
  QueryTrain,
  QueryTicket,
  QueryTransfer,
  BuyTicket,
  QueryOrder,
  RefundTicket,
  Rollback,
  Clean,
  Exit
>;

/**
 * @brief parses the command stored in str.
 *
 * this function is autogenerated.
 */
auto parse (std::string &str)
  -> Result<Command, ParseException>;

/**
 * @brief Visitor for the commands.
 *
 * The main function uses this visitor after parsing a
 * command, to actually dispatch it. Overloads of operator()
 * are callbacks of the commands.
 *
 * The implementations are in the corresponding source
 * files, not in parser.cpp.
 */
auto dispatch (const AddUser &cmd) -> void;
auto dispatch (const Login &cmd) -> void;
auto dispatch (const Logout &cmd) -> void;
auto dispatch (const QueryProfile &cmd) -> void;
auto dispatch (const ModifyProfile &cmd) -> void;
auto dispatch (const AddTrain &cmd) -> void;
auto dispatch (const ReleaseTrain &cmd) -> void;
auto dispatch (const QueryTrain &cmd) -> void;
auto dispatch (const QueryTicket &cmd) -> void;
auto dispatch (const QueryTransfer &cmd) -> void;
auto dispatch (const BuyTicket &cmd) -> void;
auto dispatch (const QueryOrder &cmd) -> void;
auto dispatch (const RefundTicket &cmd) -> void;
auto dispatch (const Rollback &cmd) -> void;
auto dispatch (const Clean &cmd) -> void;
auto dispatch (const Exit &cmd) -> void;

} // namespace ticket::command

#endif // TICKET_PARSER_H_