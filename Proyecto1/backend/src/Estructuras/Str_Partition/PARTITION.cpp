#include "PARTITION.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
using namespace std;

namespace Estructuras{

    PARTITION EmptyPartition() {
        PARTITION p{};
        memset(&p, 0, sizeof(PARTITION));
        p.Partition_status[0]= '2';
        p.Partition_type[0]= '0';
        p.Partition_fit[0] ='0';
        p.Partition_start =-1;
        p.Partition_size =-1;
        p.Partition_name[0]='0';
        p.Partition_number = 0;
        p.Partition_id[0]= '0';
        return p;
    }

    void PARTITION::CreatePartition(int partStart, int partSize, const string& partType, const string& partFit, const string& partName) {
        Partition_status[0] ='0'; //0=creada
        Partition_start= static_cast<int32_t>(partStart);
        Partition_size =static_cast<int32_t>(partSize);
        if (!partType.empty()){
            Partition_type[0]=partType[0];
        }
        
        if (!partFit.empty()) {
            Partition_fit[0]=partFit[0];
        }

        memset(Partition_name, 0, sizeof(Partition_name));
        size_t n =min(partName.size(), sizeof(Partition_name));
        memcpy(Partition_name, partName.data(), n);
    }

    void PARTITION::MountPartition(int number, const string& id) {
        Partition_status[0]='1';
        Partition_number=static_cast<int32_t>(number);
        memset(Partition_id, 0, sizeof(Partition_id));
        size_t n =min(id.size(), sizeof(Partition_id));
        memcpy(Partition_id, id.data(), n);
    }

    void PARTITION::Print() const{
        printf("---------------------- PARTITION----------------------\n");
        printf("Partition_status: %c\n", Partition_status[0]);
        printf("Partition_type: %c\n",Partition_type[0]);
        printf("Partition_fit: %c\n", Partition_fit[0]);
        printf("Partition_start: %d\n", Partition_start);
        printf("Partition_size: %d\n", Partition_size);
        printf("Partition_name: %.16s\n",Partition_name);
        printf("Partition_number: %d\n", Partition_number);
        printf("Partition_id: %.4s\n", Partition_id);
        printf("---------------------- FIN PARTITION ----------------------------\n");
    }
}