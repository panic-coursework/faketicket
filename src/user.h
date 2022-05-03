#ifndef TICKET_USER_H_
#define TICKET_USER_H_

#include "file/file.h"
#include "file/index.h"
#include "file/varchar.h"

namespace ticket {

struct User : public file::ManagedObject<User> {
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
  static auto hasUser (const char *username) -> bool;
};

extern file::File<> users;
extern file::Index<User::Id, User, decltype(users)>
  ixUsersUsername;

} // namespace ticket

#endif // TICKET_USER_H_
