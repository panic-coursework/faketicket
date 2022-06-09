#include "user.h"

#include <iostream>

#include "exception.h"
#include "hashmap.h"
#include "parser.h"
#include "rollback.h"

namespace ticket {

file::Index<User::Id, User> User::ixUsername
  {&User::username, "users.username.ix"};

/// a set of users that are logged in.
HashMap<std::string, User::Privilege> usersLoggedIn;

auto UserBase::has (const char *username) -> bool {
  return User::ixUsername.findOneId(username);
}
auto UserBase::isLoggedIn (const std::string &username)
  -> bool {
  return usersLoggedIn.contains(username);
}
auto UserBase::privilegeOf (const std::string &username)
  -> User::Privilege {
  return usersLoggedIn.at(username);
}
auto UserBase::clearSessions() -> void {
  usersLoggedIn.clear();
}

template <typename Cmd>
inline auto checkUser (const Cmd &cmd)
  -> Result<User, Exception> {
  if (!User::isLoggedIn(cmd.currentUser)) {
    return Exception("not logged in");
  }

  auto target = User::ixUsername.findOne(cmd.username);
  if (!target) return Exception("unauthorized");

  auto opPrivilege = User::privilegeOf(cmd.currentUser);
  bool insufficientPrivileges =
    opPrivilege <= target->privilege &&
    cmd.currentUser != cmd.username;
  if (insufficientPrivileges) {
    return Exception("unauthorized");
  }
  return *target;
}
inline auto makeUser (const command::AddUser &cmd) -> User {
  User user;
  user.username = cmd.username;
  user.password = cmd.password;
  user.name = cmd.name;
  user.email = cmd.email;
  return user;
}

static constexpr User::Privilege kDefaultPrivilege = 10;

auto command::run (const command::AddUser &cmd)
  -> Result<Response, Exception> {
  bool isFirstUser = User::ixUsername.empty();
  if (isFirstUser) {
    auto user = makeUser(cmd);
    user.privilege = kDefaultPrivilege;
    user.save();
    User::ixUsername.insert(user);
    rollback::log(rollback::AddUser{user.id()});
    return unit;
  }

  if (!User::isLoggedIn(cmd.currentUser)) {
    return Exception("not logged in");
  }
  auto privilegeCurr = User::privilegeOf(cmd.currentUser);
  if (privilegeCurr >= cmd.privilege) {
    return Exception("unauthorized");
  }
  if (User::has(cmd.username.data())) {
    return Exception("duplicate username");
  }

  auto user = makeUser(cmd);
  user.privilege = cmd.privilege;
  user.save();
  User::ixUsername.insert(user);
  rollback::log(rollback::AddUser{user.id()});

  return unit;
}

auto command::run (const command::Login &cmd)
  -> Result<Response, Exception> {
  if (User::isLoggedIn(cmd.username)) {
    return Exception("already logged in");
  }

  auto user = User::ixUsername.findOne(cmd.username);
  if (!user || user->password.str() != cmd.password) {
    return Exception("invalid credentials");
  }

  usersLoggedIn[cmd.username] = user->privilege;
  return unit;
}

auto command::run (const command::Logout &cmd)
  -> Result<Response, Exception> {
  if (!User::isLoggedIn(cmd.username)) {
    return Exception("not logged in");
  }
  usersLoggedIn.erase(usersLoggedIn.find(cmd.username));
  return unit;
}

auto command::run (const command::QueryProfile &cmd)
  -> Result<Response, Exception> {
  return checkUser(cmd);
}
auto command::run (const command::ModifyProfile &cmd)
  -> Result<Response, Exception> {
  auto res = checkUser(cmd);
  if (auto err = res.error()) return *err;
  auto &target = res.result();

  if (cmd.privilege) {
    bool authorized =
      *cmd.privilege < User::privilegeOf(cmd.currentUser);
    if (!authorized) return Exception("unauthorized");

    // update privilege cache if needed
    auto it = usersLoggedIn.find(cmd.username);
    if (it != usersLoggedIn.end()) {
      it->second = *cmd.privilege;
    }
  }

  rollback::ModifyProfile log;
  log.id = target.id();
#define TICKET_CHECK_FIELD(name) \
  if (cmd.name) { \
    log.name = target.name; \
    target.name = *cmd.name; \
  }
  TICKET_CHECK_FIELD(name)
  TICKET_CHECK_FIELD(email)
  TICKET_CHECK_FIELD(password)
  TICKET_CHECK_FIELD(privilege)
#undef TICKET_CHECK_FIELD
  target.update();
  // no need to update index
  rollback::log(log);

  return target;
}

auto rollback::run (const rollback::AddUser &log)
  -> Result<Unit, Exception> {
  auto user = User::get(log.id);
  User::ixUsername.remove(user);
  user.destroy();
  return unit;
}

auto rollback::run (const rollback::ModifyProfile &log)
  -> Result<Unit, Exception> {
  auto user = User::get(log.id);
  // no need to update privilege cache, since no one is
  // logged in at this point.

#define TICKET_CHECK_FIELD(name) \
  if (log.name) user.name = *log.name;
  TICKET_CHECK_FIELD(name)
  TICKET_CHECK_FIELD(email)
  TICKET_CHECK_FIELD(password)
  TICKET_CHECK_FIELD(privilege)
#undef TICKET_CHECK_FIELD
  // no need to update index
  user.update();
  return unit;
}

} // namespace ticket
