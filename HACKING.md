Hacking
=======

## Useful links

- Assignment details: <https://github.com/ACMClassCourse-2021/TicketSystem>
- Project board: <https://github.com/orgs/panic-coursework/projects/1>
- Commit message format: <https://github.com/angular/angular/blob/master/CONTRIBUTING.md#-commit-message-format>
- Class structure: <https://faketicket-docs-demo.vercel.app/>

## Internals

### Using model classes (aka Create, Read, Update, Delete)

```cpp
// Adds a user.
User u;
u.username = "Hello";
u.save();
// remember to insert into the indexes.
User::ixUsername.insert(u);

// finds a user.
User u1 = User::get(/* id */ 0);
Optional<User> yyu = User::ixUsername.findOne(
  /* username */ "yyu");
Optional<int> yyuId = User::ixUsername.findOneId("yyu");
Vector<User> yyus = User::ixUsername.findMany("yyu");
// how to use Optionals:
if (yyu) {
  std::cout << yyu->email.str() << std::endl;
} else {
  // there isn't a user named "yyu".
}

// updates a user.
u1.email = "hello@example.com";
u1.update();

// removes a user.
u.destroy();
// indexes need to be manually maintained.
User::ixUsername.remove(u);

// removes all users.
User::truncate();
User::ixUsername.truncate();
```

### Using variants

```cpp
Variant<int, bool> v1 = 1;
bool isInt = v1.is<int>(); // true
bool isBool = v1.is<bool>(); // false
int *ptr = v1.get<int>(); // points to 1
bool *ptr1 = v1.get<bool>(); // nullptr
v1 = true;
int ix = v1.index(); // 1
```

### Using results

```cpp
Result<int, Exception> res = 1926;
Result<int, Exception> res2 = Exception("error");
if (auto err = res.error()) {
  std::cout << err->what() << std::endl;
}
std::cout << res.result() << std::endl;
```

### Using optionals

```cpp
Optional<int> opt = 1;
// i.e. opt.operator bool()
TICKET_ASSERT(opt);
opt = unit;
TICKET_ASSERT(!opt);
opt = 1;
std::cout << *opt << std::endl;
```
