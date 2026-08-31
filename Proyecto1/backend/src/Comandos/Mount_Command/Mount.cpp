#include "Mount.h"
#include "../../Estructuras/Str_Mbr/MBR.h"
#include "../../Global/MountedPartitions.h"
#include "../../Utils/Utilities.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>

using namespace std;

namespace Comandos {
    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i =0; i < tokens.size(); ++i) {
            if (i>0) {
               result += " ";
            }
            result+= tokens[i];
        }
        return result;
    }

    // Estructura para almacenar los parámetros del mount
    struct MOUNT{
        string Path;
        string Name;
    };

    static bool GetPartitionId(const MOUNT& mount, int indexPartition, string& idOut, string& errMsg) {
        string letra;
        if (!Utilities::GetLetra(mount.Path, letra, errMsg)){
            errMsg= "Error obteniendo letra para el disco: "+ errMsg;
            return false;
        }
        idOut=Utilities::Carnet +to_string(indexPartition) + letra;
        errMsg.clear();
        return true;
    }

    static bool MountP(const MOUNT& mount, string& errMsg) {
        //Leer MBR
        Estructuras::MBR mbr;
        if (!mbr.DeserializeMBR(mount.Path, errMsg)) {
            errMsg = "Error al leer MBR: "+ errMsg;
            return false;
        }

        //Busca partición por nombre
        int indexPartition =-1;
        Estructuras::PARTITION* partition= mbr.GetPartitionByName(mount.Name, indexPartition, errMsg);
        if (!partition) {
            errMsg = "No se encontró la partición: " +mount.Name;
            return false;
        }

        //Solo primarias (no extendidas ni lógicas)
        if (partition->Partition_type[0] =='E' || partition->Partition_type[0] == 'L') {
            errMsg = "No se puede montar una partición extendida o lógica";
            return false;
        }

        //Si ya está montada
        if (partition->Partition_status[0]== '1') {
            errMsg = "La partición ya está montada";
            return false;
        }

        //Actualiza números de partición
        mbr.UpdatePartitionNumber();

        //Genera ID
        string id;
        if (!GetPartitionId(mount, partition->Partition_number, id, errMsg))
            return false;

        //Registra en la tabla global
        Global::MountedPartitions[id] = mount.Path;

        //Marcar partición como montada en el MBR
        partition->MountPartition(partition->Partition_number, id);

        if (!mbr.SerializeMBR(mount.Path, errMsg)) {
            errMsg = "Error al escribir MBR: " +errMsg;
            return false;
        }
        return true;
    }

    CommandResult Mount_Command(const vector<string>& tokens) {
        MOUNT mount;
        string atributos =joinTokens(tokens);
        static const regex lexic( R"(-path="[^"]+"|-path=[^\s]+|-name="[^"]+"|-name=[^\s]+)", regex::icase);

        auto begin=sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end = sregex_iterator();
        vector<string> found;
        for (auto it= begin; it!= end; ++it){
            found.push_back(it->str());
        }

        for (const auto& fun : found) {
            size_t eqPos= fun.find('=');
            if (eqPos == string::npos){
                return {false, "ERROR: Parámetro inválido: " + fun};
            }
            string key= toLowerStr(fun.substr(0, eqPos));
            string value=fun.substr(eqPos + 1);
            if (value.size() >= 2 && value.front()== '"' && value.back() == '"') {
                value = value.substr(1, value.size()-2);
            }

            if(key =="-path") {
                if (value.empty()) {
                    return {false, "ERROR: Path vaco"};
                }
                mount.Path =value;
            } 
            else if (key == "-name") {
                if(value.empty()){
                    return {false, "ERROR: Name vacío"};
                }
                mount.Name = value;
            } 
            else{
                return {false, "ERROR: Parámetro no reconocido: "+ key};
            }
        }

        if (mount.Path.empty())
            return {false, "ERROR: Falta -path"};

        if (mount.Name.empty()) {
            return {false, "ERROR: Falta -name"};
        }

        string errMsg;
        if (!MountP(mount, errMsg))
            return {false, "ERROR: "+ errMsg};

        return {true, "MOUNT: Partición montada con éxito"};
    }
}