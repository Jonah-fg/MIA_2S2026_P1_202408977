#ifndef MOUNT_COMMAND_H
#define MOUNT_COMMAND_H

#include <string>
#include <vector>
#include "../CommandResult.h"

namespace Comandos {
    CommandResult Mount_Command(const std::vector<std::string>& tokens);
}
#endif