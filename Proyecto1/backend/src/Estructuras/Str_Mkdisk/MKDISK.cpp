#include "MKDISK.h"
#include "../Str_Mbr/MBR.h"
#include "../../Utils/Utilities.h"
#include <filesystem>
#include <fstream>
#include <vector>
using namespace std;
namespace fs= std::filesystem;

namespace Estructuras  {
    bool Struct_MKDISK(const MKDISK& disk, string& errMsg) {
        long long sizeB = 0;
        if (!Utilities::ConvertBytes(disk.Size, disk.Unit, sizeB, errMsg)) {
            errMsg="Error convirtiendo el tamaño: "+ errMsg;
            return false;
        }
        if (!MakeDisk(disk, sizeB, errMsg)) {
            return false;
        }

        if (!CreateMBR(disk, sizeB, errMsg)){
            return false; 
        }
        return true;
    }

    bool MakeDisk(const MKDISK& disk, long long sizeB, string& errMsg){
        fs::path filePath(disk.Path);
        fs::path dir=filePath.parent_path();
        if (!dir.empty()){
            error_code ec;
            fs::create_directories(dir, ec);
            if (ec){ 
                errMsg= "No se pudo crear la carpeta";
                 return false; 
            }
        }
        ofstream file(disk.Path, ios::binary |ios::out | ios::trunc);
        if (!file.is_open()) { 
            errMsg ="No se pudo crear el archivo";
            return false; 
        }
        const size_t bufSize = 1024 *1024; //1 MB
        vector<char> buffer(bufSize, 0);
        long long remaining = sizeB;
        while (remaining > 0) {

            size_t wSize= (remaining< static_cast<long long>(bufSize)) ? static_cast<size_t>(remaining) : bufSize;
            file.write(buffer.data(), static_cast<streamsize>(wSize));
            if (!file) {
                {errMsg ="Error escribiendo el disco"; return false; }
            }
            remaining-= static_cast<long long>(wSize);
        }
        return true;
    }
}