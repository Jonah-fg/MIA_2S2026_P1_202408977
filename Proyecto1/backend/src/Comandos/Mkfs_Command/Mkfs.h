#ifndef MKFS_COMMAND_H
#define MKFS_COMMAND_H
#include <string>
#include <vector>
#include "../CommandResult.h"
namespace Comandos{

    struct MKFS{
        std::string Id;
        std::string Type;  
    };
    CommandResult Mkfs_Command(const std::vector<std::string>& tokens);

} 

#endif