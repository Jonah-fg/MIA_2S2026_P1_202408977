#ifndef RMGRP_COMMAND_H
#define RMGRP_COMMAND_H
#include <string>
#include "../CommandResult.h"
#include <vector>

namespace Comandos{
    CommandResult Rmgrp_Command(const std::vector<std::string>& tokens);
}
#endif