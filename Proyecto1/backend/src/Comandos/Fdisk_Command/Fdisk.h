#ifndef FDISK_COMMAND_H
#define FDISK_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos{
    CommandResult Fdisk_Command(const std::vector<std::string>& tokens);
}
#endif