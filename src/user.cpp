#include "user.h"

#include <iostream>

#include "hashmap.h"
#include "parser.h"
#include "rollback.h"
#include "strings.h"

namespace ticket {

file::Index<User::Id, User> User::ixUsername
  {&User::username, "users.username.ix"};

/// a set of users that are logged in.
HashMap<std::string, Unit> usersLoggedIn;

auto UserBase::has (const char *username) -> bool {
  return User::ixUsername.findOneId(username);
}

auto UserBase::isLoggedIn (const std::string &username)
  -> bool {
  return usersLoggedIn.contains(username);
}

inline auto isValidUsername (const std::string &username)
  -> bool {
  int length = username.length();
  if (length == 0 || length > User::Id::kMaxLength) {
    return false;
  }
  if (!isalpha(username[0])) return false;
  for (auto ch : username) {
    if (!isalnum(ch) && ch != '_') return false;
  }
  return true;
}
inline auto isValidPassword (const std::string &password)
  -> bool {
  int length = password.length();
  if (length == 0 || length > User::Password::kMaxLength) {
    return false;
  }
  for (auto ch : password) {
    if (!isVisibleChar(ch)) return false;
  }
  return true;
}
static constexpr int kNameMinLength = 2 * 3;
inline auto isValidName (const std::string &name) -> bool {
  int length = name.length();
  bool invalidLength = length < kNameMinLength
    || length > User::Name::kMaxLength;
  if (invalidLength) {
    return false;
  }
  return true;
}
inline auto isValidEmail (const std::string &email)
  -> bool {
  int length = email.length();
  if (length == 0 || length > User::Email::kMaxLength) {
    return false;
  }
  return true;
}
static constexpr int kMaxPrivilege = 10;
inline auto isValidPrivilege (int privilege) -> bool {
  return privilege >= 0 && privilege <= kMaxPrivilege;
}
inline auto isValidAddUser (const command::AddUser &cmd)
  -> bool {
  return
    isValidUsername(cmd.username) &&
    isValidPassword(cmd.password) &&
    isValidName(cmd.name) &&
    isValidEmail(cmd.email);
}

inline auto makeUser (const command::AddUser &cmd) -> User {
  User user;
  user.username = cmd.username;
  user.password = cmd.password;
  user.name = cmd.name;
  user.email = cmd.email;
  return user;
}
static constexpr int kDefaultPrivilege = 10;
auto command::dispatch (const command::AddUser &cmd)
  -> Result<Response, Exception> {
  if (!isValidAddUser(cmd)) {
    return Exception("invalid command");
  }

  bool isFirstUser = User::ixUsername.empty();
  if (isFirstUser) {
    auto user = makeUser(cmd);
    user.privilege = kDefaultPrivilege;
    user.save();
    User::ixUsername.insert(user);
    rollback::log((rollback::AddUser){(int) user.id()});
    return unit;
  }

  if (!cmd.currentUser || !cmd.privilege) {
    return Exception("insufficient privilege");
  }
  if (!User::isLoggedIn(*cmd.currentUser)) {
    return Exception("not logged in");
  }

  auto user = makeUser(cmd);
  user.save();
  User::ixUsername.insert(user);
  rollback::log((rollback::AddUser){(int) user.id()});

  return unit;
}
auto command::dispatch (const command::Login &cmd)
  -> Result<Response, Exception> {
  if (User::isLoggedIn(cmd.username)) {
    return Exception("already logged in");
  }

  auto user = User::ixUsername.findOne(cmd.username);
  if (!user || user->password.str() != cmd.password) {
    return Exception("invalid credentials");
  }

  usersLoggedIn[cmd.username] = unit;
  return unit;
}
auto command::dispatch (const command::Logout &cmd)
  -> Result<Response, Exception> {
  if (!User::isLoggedIn(cmd.username)) {
    return Exception("not logged in");
  }
  usersLoggedIn.erase(usersLoggedIn.find(cmd.username));
  return unit;
}

template <typename Cmd>
inline auto checkUser (const Cmd &cmd)
  -> Result<Pair<User, User>, Exception> {
  if (!User::isLoggedIn(cmd.currentUser)) {
    return Exception("not logged in");
  }
  auto op = User::ixUsername.findOne(cmd.currentUser);
  auto target = User::ixUsername.findOne(cmd.username);
  bool insufficientPrivileges =
    op->privilege <= target->privilege &&
    op->id() != target->id();
  if (!target || insufficientPrivileges) {
    return Exception("unauthorized");
  }
  return Pair(*target, *op);
}
auto command::dispatch (const command::QueryProfile &cmd)
  -> Result<Response, Exception> {
  auto res = checkUser(cmd);
  if (auto err = res.error()) return *err;
  auto [ target, _ ] = res.result();
  return target;
}
auto command::dispatch (const command::ModifyProfile &cmd)
  -> Result<Response, Exception> {
  auto res = checkUser(cmd);
  if (auto err = res.error()) return *err;
  auto [ target, op ] = res.result();

  bool sufficientPrivileges = cmd.privilege < op.privilege;
  if (!sufficientPrivileges) {
    return Exception("insufficient privileges");
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

auto rollback::dispatch (const rollback::AddUser &log)
  -> Result<Unit, Exception> {
  auto user = User::get(log.id);
  User::ixUsername.remove(user);
  user.destroy();
  return unit;
}
auto rollback::dispatch (const rollback::ModifyProfile &log)
  -> Result<Unit, Exception> {
  auto user = User::get(log.id);
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
