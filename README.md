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
Frontend: BLeaves    

## Code Structure

`lib/` contains library stuff unrelated to business logic.
It is written to replace many useful utilities in the C++
STL, and to implement file storage.

`src/` contains business logic of the ticket system. The
main function performs a Read-Eval-Print Loop (REPL).
The command parser parses the command string into command
arguments, and main dispatches the command.

For the detailed design, see `docs/refman.pdf` or
<https://faketicket-docs-demo.vercel.app/>, both generated
by [Doxygen](https://www.doxygen.nl/). They contain the
same contents. You can generate these docs by running
`doxygen` in the root directory of this repository.

## Database files

- `orders`: orders
- `orders.user.ix`: index of orders on username
- `pending-orders`: pending orders
- `pending-orders.ride.ix`: index of pending orders on ride
  (i.e. train ID and date)
- `rollback-log`: rollback log
- `trains`: trains
- `trains.train-id.ix`: index of trains on train ID
- `trains.stop.ix`: index of trains on station names
- `ride-seats`: remaining seat count information
- `ride-seats.ride.ix`: index of ride-seats on ride
- `users`: users
- `users.username.ix`: index of users on username

## License

SPDX-License-Identifier: GPL-3.0-or-later
