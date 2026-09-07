#ifndef FILEBLOCK_H
#define FILEBLOCK_H

#include <string>

namespace Estructuras{

#pragma pack(push, 1)
    struct FILEBLOCK {
        char B_content[64]; //(64 bytes)

        bool Serialize(const std::string& path, long long offset, std::string& errMsg);
        bool Deserialize(const std::string& path, long long offset, std::string& errMsg);
        void Print() const;
    };
#pragma pack(pop)

} 

#endif