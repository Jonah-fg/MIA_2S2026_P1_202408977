#include "FOLDERBLOCK.h"
#include <cstdio>
#include <fstream>
using namespace std;
namespace Estructuras{

    bool FOLDERBLOCK::Serialize(const string& path, long long offset, string& errMsg) {
        fstream file(path, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            errMsg = "ERROR: No se pudo abrir el archiv al serializar el folderblock";
            return false;
        }
        file.seekp(offset, ios::beg);
        file.write(reinterpret_cast<const char*>(this), sizeof(FOLDERBLOCK));
        if (!file){
            errMsg = "ERROR: No se pudo escribir el folderblock en el archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }

    bool FOLDERBLOCK::Deserialize(const string& path, long long offset, string& errMsg) {
        ifstream file(path, ios::binary);
        if (!file.is_open()) {
            errMsg ="ERROR: No se pudo abrir el archivo al deserializar el folderblock";
            return false;
        }
        file.seekg(offset, ios::beg);
        file.read(reinterpret_cast<char*>(this), sizeof(FOLDERBLOCK));
        if (!file){
            errMsg="ERROR: No se pudo leer el folderblock del archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }

    void FOLDERBLOCK::Print() const {
        for (int i = 0; i < 4; ++i) {
            printf("Contenido %d:\n", i+1);
            printf("\tB_name: %.12s\n", B_content[i].B_name);
            printf("\tB_inodo: %d\n", B_content[i].B_inodo);
        }
    }
} 