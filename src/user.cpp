#include "user.h"

#include <iostream>

#include "hashmap.h"
#include "parser.h"
#include "rollback.h"

namespace ticket {

file::Index<User::Id, User> User::ixUsername
  {&User::username, "users.username.ix"};

/// a set of users that are logged in.
HashMap<std::string, Unit> usersLoggedIn;

auto UserBase::hasUser (const char *username) -> bool {
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
