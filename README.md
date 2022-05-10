Faketicket
==========

SJTU CS1951 lab assignment

Assignment details:
<https://github.com/ACMClassCourse-2021/TicketSystem>

## Code Owners

Overall structure: Alan-Liang    
Library utilities: Alan-Liang    
Misc, Order, User, Parser: Alan-Liang    
Rollback, Train: BLeaves    
Frontend:    
&nbsp; UI &amp; API Design, Implementation, BFF: BLeaves    
&nbsp; Node-C++ interop: Alan-Liang

## Code Structure

`lib/` contains library stuff unrelated to business logic.
It is written to replace many useful utilities in the C++
STL, and to implement file storage.

`src/` contains business logic of the ticket system. The
main function performs a Read-Eval-Print Loop (REPL).
The node module provides an interface to the Node platform.
The command parser parses the command string into command
arguments, and main dispatches the command.

Frontend code is to be written in JavaScript utilizing the
Vue.js library. A backend-for-frontend (BFF), also written
in JavaScript and to be ran in the Node runtime, acts as the
intermediary between frontend API calls and the C++ backend.
It also serves static assets. Optionally, a reverse proxy
could be used to achieve load balancing, HTTPS, logging or
other purposes.

The frontend would be a single-page application (SPA). The
frontend code would fetch raw data in JSON format from the
BFF, and turn it into a DOM structure to be rendered in the
browser. Detailed UI/UX design is TBD.

For the detailed code structure, see `docs/refman.pdf` or
<https://faketicket-docs-demo.vercel.app/>, both generated
by [Doxygen](https://www.doxygen.nl/). They contain the
same contents. You can generate these docs by running
`doxygen` in the root directory of this repository.

## Database files

- `orders`: orders
- `orders.user.ix`: index of orders on username
- `orders-pending.ride.ix`: index of pending orders on ride
  (i.e. train ID and date)
- `rollback-log`: rollback log
- `trains`: trains
- `trains.train-id.ix`: index of trains on train ID
- `trains.stop.ix`: index of trains on station names
- `ride-seats`: remaining seat count information
- `ride-seats.ride.ix`: index of ride-seats on ride
- `users`: users
- `users.username.ix`: index of users on username

### Data flow

```plain
+-----------------------------------------+
|  Filesystem                      Server |
|     ^                                   |
|     | Filesystem APIs                   |
|     v                                   |
| C++ "./code"                            |
|     ^                                   |
|     | Node-API                          |
|     v                                   |
| BFF (Backend for frontend)       Server |
+-----^-----------------------------------+
|     | http(s)                   Network |
+-----v-----------------------------------+
|  Browser                         Client |
+-----------------------------------------+
```

## License

SPDX-License-Identifier: GPL-3.0-or-later
