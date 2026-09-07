#include "SUPERBLOCK.h"
#include <fstream>
using namespace std;

namespace Estructuras{

    bool SUPERBLOCK::Serialize(const string& path, long long offset, string& errMsg) {
        fstream file(path, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            errMsg ="ERROR: No se pudo abrir el archivo al serializar el superbloque";
            return false;
        }
        file.seekp(offset, ios::beg);
        file.write(reinterpret_cast<const char*>(this), sizeof(SUPERBLOCK));
        if (!file){
            errMsg = "ERROR: No se pudo escribir el superloque en el archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }

    bool SUPERBLOCK::Deserialize(const string& path, long long offset, string& errMsg) {
        ifstream file(path, ios::binary);
        if (!file.is_open()) {
            errMsg = "ERROR: No se pudo abrir el archivo al deserializar el superbloque";
            return false;
        }
        file.seekg(offset, ios::beg);
        file.read(reinterpret_cast<char*>(this), sizeof(SUPERBLOCK));
        if (!file){
            errMsg ="ERROR: No se pudo leer el superbloque del archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }
} 