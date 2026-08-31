#ifndef COMMAND_RESULT_H
#define COMMAND_RESULT_H
#include <string>

namespace Comandos {
    struct CommandResult {
        bool success = false;
        std::string message;
    };
}
#endif