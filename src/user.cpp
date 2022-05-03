#include "user.h"

#include <iostream>

#include "hashmap.h"
#include "parser.h"
#include "rollback.h"

namespace ticket {

file::File<> users {"users"};
file::Index<User::Id, User, decltype(users)> ixUsersUsername
  {&User::username, "users.username.ix", users};

/// a set of users that are logged in.
HashMap<std::string, Unit> usersLoggedIn;

auto User::hasUser (const char *username) -> bool {
  // TODO
}


auto command::dispatch (const command::AddUser &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::Login &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::Logout &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::QueryProfile &cmd) -> void {
  // TODO
}
auto command::dispatch (const command::ModifyProfile &cmd) -> void {
  // TODO
}

auto rollback::dispatch (const rollback::AddUser &log) -> void {
  // TODO
}
auto rollback::dispatch (const rollback::ModifyProfile &log) -> void {
  // TODO
}

} // namespace ticket
