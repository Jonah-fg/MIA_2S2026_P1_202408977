#ifndef LOGIN_COMMAND_H
#define LOGIN_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos{
    CommandResult Login_Command(const std::vector<std::string>& tokens);
}
#endif