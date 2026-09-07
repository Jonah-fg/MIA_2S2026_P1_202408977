#include "FILEBLOCK.h"
#include <cstdio>
#include <fstream>
using namespace std;

namespace Estructuras
{

    bool FILEBLOCK::Serialize(const std::string &path, long long offset, std::string &errMsg)
    {
        std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
        if (!file.is_open())
        {
            errMsg ="ERROR: No se pudo abrir el archivo al serializar el fileblock";
            return false;
        }

        file.seekp(offset, std::ios::beg);
        file.write(reinterpret_cast<const char *>(this), sizeof(FILEBLOCK));

        if (!file)
        {
            errMsg = "ERROR: No se pudo escribir el fileblock en el archivo";
            return false;
        }

        errMsg.clear();
        return true;
    }

    bool FILEBLOCK::Deserialize(const std::string &path, long long offset, std::string &errMsg)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()){
            errMsg = "ERROR: No se pudo abrir el archivo al deserializar el fileblock";
            return false;
        }

        file.seekg(offset, std::ios::beg);
        file.read(reinterpret_cast<char *>(this), sizeof(FILEBLOCK));

        if (!file)
        {
            errMsg = "ERROR: No se pudo leer el fileblock del archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }

    void FILEBLOCK::Print() const
    {
        // %.64s limita la impresion a los 64 bytes del arreglo, por si no
        // hay un byte nulo de terminacion entre el contenido real
        std::printf("%.64s", B_content);
    }

}