// This is the entrypoint of the backend program.
#include <iostream>

#include "parser.h"
#include "response.h"
#include "rollback.h"
#include "run.h"
#include "utility.h"

auto main () -> int {
#ifdef ONLINE_JUDGE
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
#endif // ONLINE_JUDGE

  while (true) {
    // parse timestamp
    char c1 = (char) std::cin.get();
    TICKET_ASSERT(c1 == '[');
    int timestamp;
    std::cin >> timestamp;
    ticket::setTimestamp(timestamp);
    char c2 = (char) std::cin.get();
    TICKET_ASSERT(c2 == ']');
    std::cout << '[' << timestamp << "] ";

    // main command
    std::string input;
    std::getline(std::cin, input);
    auto cmd = ticket::command::parse(input);
    if (auto err = cmd.error()) {
      std::cerr << err->what() << std::endl;
      return 1;
    }

    cmd.result().visit([] (const auto &args) {
      auto res = ticket::command::run(args);
      if (res.error()) {
        std::cout << "-1\n";
      } else {
        res.result().visit([] (const auto &res) {
          ticket::response::cout(res);
        });
      }
    });
  }
}
