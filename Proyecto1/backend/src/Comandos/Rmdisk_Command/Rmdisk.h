#ifndef RMDISK_COMMAND_H
#define RMDISK_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos {
    CommandResult Rmdisk_Command(const std::vector<std::string>& tokens);
}
#endif