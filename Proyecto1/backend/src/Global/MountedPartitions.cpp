#include "MountedPartitions.h"

namespace Global{
    std::unordered_map<std::string, std::string> MountedPartitions;

    bool GetMountedPartition(const string& id, Estructuras::PARTITION& partOut, string& diskPathOut, string& errMsg) {
        auto it=MountedPartitions.find(id);
        if(it ==MountedPartitions.end() || it->second.empty()) {
            errMsg = "La partición con ID '" +id+ "' no está montada";
            return false;
        }
        string path = it->second;
        Estructuras::MBR mbr;
        if (!mbr.DeserializeMBR(path, errMsg)){
            errMsg = "Error al leer MBR del disco: "+ errMsg;
            return false;
        }
        const Estructuras::PARTITION* part = mbr.GetPartitionByID(id, errMsg);
        if (!part){
            errMsg="No se encontró la partición con ID '" + id + "'en el MBR";
            return false;
        }
        partOut= *part;
        diskPathOut= path;
        errMsg.clear();
        return true;
    }


    bool GetEssentialRep(const std::string& id, Estructuras::MBR& mbrOut, std::string& diskPathOut, std::string& errMsg) {
        auto it= MountedPartitions.find(id);
        if (it == MountedPartitions.end() || it->second.empty()) {
            errMsg ="ID de partición no montada: " + id;
            return false;
        }
        std::string path=it->second;
        if (!mbrOut.DeserializeMBR(path, errMsg)){
            errMsg = "Error al leer MBR del disco: "+ errMsg;
            return false;
        }
        diskPathOut = path;
        errMsg.clear();
        return true;
    }
}