#ifndef MKUSR_COMMAND_H
#define MKUSR_COMMAND_H

#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos {
    CommandResult Mkusr_Command(const std::vector<std::string>& tokens);
}
#endif