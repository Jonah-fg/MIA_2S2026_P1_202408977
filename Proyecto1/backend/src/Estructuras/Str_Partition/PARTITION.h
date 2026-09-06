#pragma once
#include <cstdint>
#include <string>
using namespace std;

namespace Estructuras {
#pragma pack(push, 1)
    struct PARTITION {
        char Partition_status[1];
        char Partition_type[1];
        char Partition_fit[1];
        int32_t Partition_start;
        int32_t Partition_size;
        char Partition_name[16];
        int32_t Partition_number;
        char Partition_id[4];

        void Print() const;
        void CreatePartition(int partStart, int partSize, const string& partType, const string& partFit, const string& partName);
        void MountPartition(int number, const string& id);
    };
#pragma pack(pop)

    // Devuelve una partición vacía 
    PARTITION EmptyPartition();
}