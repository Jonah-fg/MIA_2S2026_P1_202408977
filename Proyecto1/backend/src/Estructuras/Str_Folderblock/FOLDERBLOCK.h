#ifndef FOLDERBLOCK_H
#define FOLDERBLOCK_H

#include <string>
#include <cstdint>

namespace Estructuras {

#pragma pack(push, 1)
    struct FOLDERCONTENT {
        char B_name[12]; // nombre del archivo/carpeta
        int32_t B_inodo;   // número de inodo al que apunta
    };
#pragma pack(pop)
#pragma pack(push, 1)
    struct FOLDERBLOCK {
        FOLDERCONTENT B_content[4];  

        bool Serialize(const std::string& path, long long offset, std::string& errMsg);
        bool Deserialize(const std::string& path, long long offset, std::string& errMsg);
        void Print() const;
    };
#pragma pack(pop)

} 
#endif