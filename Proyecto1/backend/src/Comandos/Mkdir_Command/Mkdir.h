#ifndef MKDIR_COMMAND_H
#define MKDIR_COMMAND_H

#include <string>
#include <vector>
#include "../CommandResult.h"
namespace Comandos {
    CommandResult Mkdir_Command(const std::vector<std::string>& tokens);
}
#endif