#include "INODE.h"
#include <cstdio>
#include <ctime>
#include <fstream>
using namespace std;

namespace Estructuras{

    bool INODE::Serialize(const string& path, long long offset, string& errMsg) {
        fstream file(path, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            errMsg ="ERROR: No se pudo abrir el archivo al serializar el inodo";
            return false;
        }
        file.seekp(offset, ios::beg);
        file.write(reinterpret_cast<const char*>(this), sizeof(INODE));
        if (!file) {
            errMsg = "ERROR: No se pudo escribir el inodo en el archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }

    bool INODE::Deserialize(const string& path, long long offset, string& errMsg) {
        ifstream file(path, ios::binary);
        if (!file.is_open()) {
            errMsg = "ERROR: No se pudo abrir el archivo al deserializar el inodo";
            return false;
        }
        file.seekg(offset, ios::beg);
        file.read(reinterpret_cast<char*>(this), sizeof(INODE));
        if (!file) {
            errMsg = "ERROR: No se pudo leer el inodo del archivo";
            return false;
        }
        errMsg.clear();
        return true;
    }

    void INODE::Print() const {
        auto formatTime = [](float unixTime, char* outBuf, size_t outSize) {
            time_t t = static_cast<time_t>(unixTime);
            struct tm tmResult;
            gmtime_r(&t, &tmResult);
            strftime(outBuf, outSize, "%Y-%m-%dT%H:%M:%SZ", &tmResult);
        };

        char atimeBuf[32], ctimeBuf[32], mtimeBuf[32];
        formatTime(I_atime, atimeBuf, sizeof(atimeBuf));
        formatTime(I_ctime, ctimeBuf, sizeof(ctimeBuf));
        formatTime(I_mtime, mtimeBuf, sizeof(mtimeBuf));

        printf("I_uid: %d\n", I_uid);
        printf("I_gid: %d\n", I_gid);
        printf("I_size: %d\n", I_size);
        printf("I_atime: %s\n", atimeBuf);
        printf("I_ctime: %s\n", ctimeBuf);
        printf("I_mtime: %s\n", mtimeBuf);
        printf("I_block: [");
        for (int i = 0; i < 15; ++i) {
            printf("%d", I_block[i]);
            if (i < 14) printf(" ");
        }
        printf("]\n");
        printf("I_type: %.1s\n", I_type);
        printf("I_perm: %.3s\n", I_perm);
    }

} // namespace Structs