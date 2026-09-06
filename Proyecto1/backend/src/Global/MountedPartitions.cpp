#include "MountedPartitions.h"

namespace Global{
    std::unordered_map<std::string, std::string> MountedPartitions;

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