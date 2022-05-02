// This file is autogenerated. Do not modify.
#include "parser.h"

#include "utility.h"

namespace ticket::command {

auto parseCommand (std::string &str)
  -> Result<Command, ParseException> {
  auto argv = split(str, ' ');
  auto &argv0 = argv[0];
  if (argv0 == "add_user") {
    AddUser res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-c") {
        res.currentUser = argv[++i].data();
      } else if (arg == "-u") {
        res.username = argv[++i].data();
      } else if (arg == "-p") {
        res.password = argv[++i].data();
      } else if (arg == "-n") {
        res.name = argv[++i].data();
      } else if (arg == "-m") {
        res.email = argv[++i].data();
      } else if (arg == "-g") {
        res.privilege = atoi(argv[++i].data());
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "login") {
    Login res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-u") {
        res.username = argv[++i].data();
      } else if (arg == "-p") {
        res.password = argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "logout") {
    Logout res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-u") {
        res.username = argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "query_profile") {
    QueryProfile res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-c") {
        res.currentUser = argv[++i].data();
      } else if (arg == "-u") {
        res.username = argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "modify_profile") {
    ModifyProfile res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-c") {
        res.currentUser = argv[++i].data();
      } else if (arg == "-u") {
        res.username = argv[++i].data();
      } else if (arg == "-p") {
        res.password = argv[++i].data();
      } else if (arg == "-n") {
        res.name = argv[++i].data();
      } else if (arg == "-m") {
        res.email = argv[++i].data();
      } else if (arg == "-g") {
        res.privilege = atoi(argv[++i].data());
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "add_train") {
    AddTrain res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-i") {
        res.id = argv[++i].data();
      } else if (arg == "-n") {
        res.stops = atoi(argv[++i].data());
      } else if (arg == "-m") {
        res.seats = atoi(argv[++i].data());
      } else if (arg == "-s") {
        std::string val = argv[++i].data();
        res.stations = copyStrings(split(val, '|'));
      } else if (arg == "-p") {
        std::string val = argv[++i].data();
        auto values = split(val, '|');
        res.prices.reserve(values.size());
        for (auto &str : values) {
          res.prices.push_back(atoi(str.data()));
        }
      } else if (arg == "-x") {
        res.departure = Instant(argv[++i].data());
      } else if (arg == "-t") {
        std::string val = argv[++i].data();
        auto values = split(val, '|');
        res.durations.reserve(values.size());
        for (auto &str : values) {
          res.durations.push_back(Duration(str.data()));
        }
      } else if (arg == "-o") {
        std::string val = argv[++i].data();
        auto values = split(val, '|');
        res.stopoverTimes.reserve(values.size());
        for (auto &str : values) {
          res.stopoverTimes.push_back(Duration(str.data()));
        }
      } else if (arg == "-d") {
        std::string val = argv[++i].data();
        auto values = split(val, '|');
        res.dates.reserve(values.size());
        for (auto &str : values) {
          res.dates.push_back(Date(str.data()));
        }
      } else if (arg == "-y") {
        res.type = *argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "release_train") {
    ReleaseTrain res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-i") {
        res.id = argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "query_train") {
    QueryTrain res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-i") {
        res.id = argv[++i].data();
      } else if (arg == "-d") {
        res.date = Date(argv[++i].data());
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "query_ticket") {
    QueryTicket res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-s") {
        res.from = argv[++i].data();
      } else if (arg == "-t") {
        res.to = argv[++i].data();
      } else if (arg == "-d") {
        res.date = Date(argv[++i].data());
      } else if (arg == "-p") {
        res.sort = argv[++i].data()[0] == 't' ? kTime : kCost;
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "query_transfer") {
    QueryTransfer res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-s") {
        res.from = argv[++i].data();
      } else if (arg == "-t") {
        res.to = argv[++i].data();
      } else if (arg == "-d") {
        res.date = Date(argv[++i].data());
      } else if (arg == "-p") {
        res.sort = argv[++i].data()[0] == 't' ? kTime : kCost;
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "buy_ticket") {
    BuyTicket res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-u") {
        res.currentUser = argv[++i].data();
      } else if (arg == "-i") {
        res.train = argv[++i].data();
      } else if (arg == "-d") {
        res.date = Date(argv[++i].data());
      } else if (arg == "-n") {
        res.seats = atoi(argv[++i].data());
      } else if (arg == "-f") {
        res.from = argv[++i].data();
      } else if (arg == "-t") {
        res.to = argv[++i].data();
      } else if (arg == "-q") {
        res.queue = argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "query_order") {
    QueryOrder res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-u") {
        res.currentUser = argv[++i].data();
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "refund_ticket") {
    RefundTicket res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-u") {
        res.currentUser = argv[++i].data();
      } else if (arg == "-n") {
        res.index = atoi(argv[++i].data());
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "rollback") {
    Rollback res;
    for (int i = 1; i < argv.size(); ++i) {
      auto &arg = argv[i];
      if (arg == "-t") {
        res.timestamp = atoi(argv[++i].data());
      } else {
        return ParseException();
      }
    }
    return Command(res);
  } else if (argv0 == "clean") {
    return Command(Clean());
  } else if (argv0 == "exit") {
    return Command(Exit());
  } else {
    return ParseException();
  }
}

} // namespace ticket::command
