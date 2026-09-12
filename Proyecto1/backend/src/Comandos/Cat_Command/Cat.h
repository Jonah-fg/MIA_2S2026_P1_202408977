#ifndef CAT_COMMAND_H
#define CAT_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos {
    CommandResult Cat_Command(const std::vector<std::string>& tokens);
}
#endif