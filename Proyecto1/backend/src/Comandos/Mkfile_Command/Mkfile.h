#ifndef MKFILE_COMMAND_H
#define MKFILE_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"
namespace Comandos {
    CommandResult Mkfile_Command(const std::vector<std::string>& tokens);
}
#endif