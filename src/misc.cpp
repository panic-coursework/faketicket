// This file implements the clean and exit commands.
#include "parser.h"

namespace ticket {

auto command::dispatch (const command::Clean & /* unused */) -> Result<Response, Exception> {
  // TODO
}
auto command::dispatch (const command::Exit & /* unused */) -> Result<Response, Exception> {
  // TODO
}

} // namespace ticket

