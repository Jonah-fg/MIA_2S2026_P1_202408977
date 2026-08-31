#ifndef MKDISK_COMMAND_H
#define MKDISK_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos{
    CommandResult Mkdisk_Command(const std::vector<std::string>& tokens);
}
#endif