#ifndef MKGRP_COMMAND_H
#define MKGRP_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos{
    CommandResult Mkgrp_Command(const std::vector<std::string>& tokens);
}
#endif