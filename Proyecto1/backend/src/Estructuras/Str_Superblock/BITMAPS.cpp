#include "SUPERBLOCK.h"
#include <fstream>
#include <vector>
using namespace std;

namespace Estructuras{
    bool SUPERBLOCK::Create_Bit_Maps(const string& path, string& errMsg) {
        fstream file(path, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            errMsg ="ERROR: No se pdo abrir el archivo del disco";
            return false;
        }

        const int totalInodos = Sb_inodes_count + Sb_free_inodes_count;
        const int totalBloques = Sb_blocks_count + Sb_free_blocks_count;

        //Bitmap de inodos
        file.seekp(Sb_bm_inode_start, ios::beg);
        vector<char> inodeBitmap(static_cast<size_t>(totalInodos), '0');
        file.write(inodeBitmap.data(), static_cast<streamsize>(inodeBitmap.size()));
        if (!file){
            errMsg ="ERROR: No se pudo escribir el bitmap de inodos";
            return false;
        }

        //Bitmap de bloques
        file.seekp(Sb_bm_block_start, ios::beg);
        vector<char> blockBitmap(static_cast<size_t>(totalBloques), '0');
        file.write(blockBitmap.data(), static_cast<streamsize>(blockBitmap.size()));
        if(!file){
            errMsg ="ERROR: No se pudo escribir el bitmap de bloques";
            return false;
        }
        errMsg.clear();
        return true;
    }

    bool SUPERBLOCK::Update_Inode_Bitmap(const string& path, string& errMsg) {
        fstream file(path, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            errMsg= "ERROR: No se pudo abrir el archivo del disco";
            return false;
        }
        long long pos=static_cast<long long>(Sb_bm_inode_start) + static_cast<long long>(Sb_inodes_count);
        file.seekp(pos, ios::beg);
        char bit ='1';
        file.write(&bit, 1);
        if (!file){
            errMsg= "ERROR: No se pudo actualizar el bitmap de inodos";
            return false;
        }
        errMsg.clear();
        return true;
    }

    bool SUPERBLOCK::Update_Block_Bitmap(const string& path, string& errMsg) {
        fstream file(path, ios::binary |ios::in | ios::out);
        if (!file.is_open()){
            errMsg= "ERROR: No se pudo abrir el archivo del disco";
            return false;
        }
        long long pos=static_cast<long long>(Sb_bm_block_start)+ static_cast<long long>(Sb_blocks_count);
        file.seekp(pos, ios::beg);
        char bit ='1';
        file.write(&bit, 1);
        if (!file){
            errMsg ="ERROR: No se pudo actualizar el bitmap de bloques";
            return false;
        }
        errMsg.clear();
        return true;
    }
}