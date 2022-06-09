#ifndef TICKET_USER_H_
#define TICKET_USER_H_

#include "file/file.h"
#include "file/index.h"
#include "file/varchar.h"

namespace ticket {

struct UserBase {
  using Id = file::Varchar<20>;
  using Password = file::Varchar<30>;
  using Name = file::Varchar<15>;
  using Email = file::Varchar<30>;
  using Privilege = int;

  Id username;
  Password password;
  Name name;
  Email email;
  Privilege privilege;

  /// checks if there is a user with the given username.
  static auto has (const char *username) -> bool;
  /// checks if the user is logged in.
  static auto isLoggedIn (const std::string &username)
    -> bool;
  /// returns the privilege of a user. The user has to be
  /// logged in.
  static auto privilegeOf (const std::string &username)
    -> int;
  /// logs out all the users.
  static auto clearSessions () -> void;

  static constexpr const char *filename = "users";
};
struct User : public file::Managed<UserBase> {
  User () = default;
  User (const file::Managed<UserBase> &user)
    : file::Managed<UserBase>(user) {}
  static file::Index<User::Id, User> ixUsername;
};

} // namespace ticket

#endif // TICKET_USER_H_
