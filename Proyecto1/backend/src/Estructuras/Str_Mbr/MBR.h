#pragma once
#include <cstdint>
#include <string>
#include "../Str_Partition/PARTITION.h"
#include "../Str_Mkdisk/MKDISK.h"
using namespace std;

namespace Estructuras{
#pragma pack(push, 1)
    struct MBR {
        int32_t Mbr_size;
        float Mbr_date;
        int32_t Mbr_signature_disk;
        char Mbr_disk_fit[1];
        PARTITION Mbr_partitions[4];

        bool SerializeMBR(const string& path, string& errMsg);
        bool DeserializeMBR(const string& path, string& errMsg);
        PARTITION* GetFirstPartitionAvailable(int& startOut, int& indexOut, string& errMsg);
        const PARTITION* GetPartitionByID(const string& id, string& errMsg) const;
        PARTITION* GetPartitionByName(const string& name, int& indexOut, string& errMsg);
        void UpdatePartitionNumber();
        void Print() const;
    };
#pragma pack(pop)

    bool CreateMBR(const MKDISK& disk, long long sizeB, string& errMsg);
}