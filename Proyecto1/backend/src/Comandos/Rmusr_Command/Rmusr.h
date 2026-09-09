#ifndef RMUSR_COMMAND_H
#define RMUSR_COMMAND_H

#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos{
    CommandResult Rmusr_Command(const std::vector<std::string>& tokens);
}
#endif