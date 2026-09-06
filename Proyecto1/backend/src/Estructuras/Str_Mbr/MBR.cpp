#include "MBR.h"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <random>
using namespace std;

namespace Estructuras{
    namespace {
        bool equalFold(const string& a, const string& b) {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); ++i) {
                if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
                    return false;
            }
            return true;
        }
        string trimNulBoth(const string& s) {
            size_t start = s.find_first_not_of('\0');
            if (start == string::npos) return "";
            size_t end = s.find_last_not_of('\0');
            return s.substr(start, end - start + 1);
        }
        int32_t randomSignature(){
            static mt19937 rng(random_device{}());
            static uniform_int_distribution<int32_t> dist(0, 2147483647);
            return dist(rng);
        }
    }

    bool CreateMBR(const MKDISK& disk, long long sizeB, string& errMsg) {
        char fByte;
        if (disk.Fit == "BF")
        fByte = 'B';

        else if (disk.Fit == "FF") 
        fByte = 'F';

        else if(disk.Fit == "WF")
         fByte = 'W';

        else { 
            errMsg = "Ajuste no reconocido"; return false;
       }

        MBR mbr{};
        memset(&mbr, 0, sizeof(MBR));
        mbr.Mbr_size = static_cast<int32_t>(sizeB);
        mbr.Mbr_date = static_cast<float>(time(nullptr));
        mbr.Mbr_signature_disk =randomSignature();
        mbr.Mbr_disk_fit[0] = fByte;
        for (int i = 0; i<4; ++i)
            mbr.Mbr_partitions[i] = EmptyPartition();
        if (!mbr.SerializeMBR(disk.Path, errMsg))
            return false;

        return true;
    }

    bool MBR::SerializeMBR(const string& path, string& errMsg) {
        fstream file(path, ios::binary | ios::in | ios::out);
        if (!file.is_open()) { errMsg = "No se pudo abrir el archivo"; return false; }
        file.seekp(0);
        file.write(reinterpret_cast<const char*>(this), sizeof(MBR));
        if (!file) { errMsg = "No se pudo escribir el MBR"; return false; }
        return true;
    }

    bool MBR::DeserializeMBR(const string& path, string& errMsg) {
        ifstream file(path, ios::binary);
        if (!file.is_open()) { errMsg = "No se pudo abrir para leer"; return false; }
        file.read(reinterpret_cast<char*>(this), sizeof(MBR));
        if (!file) { errMsg = "No se pudo leer el MBR"; return false; }
        return true;
    }

    PARTITION* MBR::GetFirstPartitionAvailable(int& startOut, int& indexOut, string& errMsg) {
        int offset=static_cast<int>(sizeof(MBR));
        for (int i= 0; i < 4; ++i) {
            if (Mbr_partitions[i].Partition_start == -1) {
                startOut=offset;
                indexOut =i;
                errMsg.clear();
                return &Mbr_partitions[i];
            } 
            else {
                offset+= Mbr_partitions[i].Partition_size;
            }
        }
        startOut=-1; indexOut = -1;
        errMsg.clear();
        return nullptr;
    }

    const PARTITION* MBR::GetPartitionByID(const string& id, string& errMsg) const {
        string inputID = trimNulBoth(id);
        for (int i = 0; i < 4; ++i) {
            const PARTITION& p = Mbr_partitions[i];
            string partitionID(p.Partition_id, sizeof(p.Partition_id));
            partitionID = trimNulBoth(partitionID);
            if (equalFold(partitionID, inputID)) {
                errMsg.clear();
                return &p;
            }
        }
        errMsg = "No se encontró la partición con id: " + id;
        return nullptr;
    }

    PARTITION* MBR::GetPartitionByName(const string& name, int& indexOut, string& errMsg) {
        string inputName = trimNulBoth(name);
        for (int i =0; i <4; ++i) {
            PARTITION& p=Mbr_partitions[i];
            string partitionName(p.Partition_name, sizeof(p.Partition_name));
            partitionName = trimNulBoth(partitionName);
            if (equalFold(partitionName, inputName)) {
                indexOut = i;
                errMsg.clear();
                return &p;
            }
        }
        indexOut = -1;
        errMsg = "No se encontró la partición con nombre: " + name;
        return nullptr;
    }

    void MBR::UpdatePartitionNumber() {
        int number = 1;
        for (int i = 0; i < 4; ++i) {
            PARTITION& partition = Mbr_partitions[i];
            if (partition.Partition_status[0] != 0 && partition.Partition_type[0] == 'P') {
                partition.Partition_number = number++;
            } else if (partition.Partition_type[0] == 'E') {
                partition.Partition_number = 0;
            }
        }
    }

    void MBR::Print() const {
        printf("---------- MBR ----------\n");
        printf("Mbr_size: %d\n", Mbr_size);
        printf("Mbr_date: %.0f\n", Mbr_date);
        printf("Mbr_signature_disk: %d\n", Mbr_signature_disk);
        printf("Mbr_disk_fit: %c\n", Mbr_disk_fit[0]);
        printf("---------- END MBR ----------\n");
    }
}