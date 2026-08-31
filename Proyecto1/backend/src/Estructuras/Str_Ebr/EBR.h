#pragma once
#include <cstdint>
using namespace std;

namespace Estructuras {
#pragma pack(push, 1)
    struct EBR{
        char Partition_mount[1];
        char Partition_fit[1];
        int32_t Partition_start;
        int32_t Partition_size;
        int32_t Partition_next;
        char Partition_name[16];
    };
#pragma pack(pop)
}