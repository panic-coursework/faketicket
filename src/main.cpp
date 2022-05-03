// This is the entrypoint of the backend program.
#include <iostream>

#include "parser.h"

auto main () -> int {
  while (true) {
    std::string input;
    std::getline(std::cin, input);
    auto cmd = ticket::command::parse(input);
    if (auto err = cmd.error()) {
      std::cerr << err->what() << std::endl;
      return 1;
    }
    cmd.result().visit([] (const auto &args) {
      ticket::command::dispatch(args);
    });
  }
}
