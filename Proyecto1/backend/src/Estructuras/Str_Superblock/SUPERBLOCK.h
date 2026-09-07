#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include <string>
#include <cstdint>

namespace Estructuras {

#pragma pack(push, 1)
    struct SUPERBLOCK{
        int32_t Sb_filesystem_type;  
        int32_t Sb_inodes_count;   //cantidad de inodos usados
        int32_t Sb_blocks_count;    //cantidad de bloques usados
        int32_t Sb_free_blocks_count; //bloques libres
        int32_t Sb_free_inodes_count; // inodos libres
        float Sb_mtime;             //fecha último montaje
        float Sb_umtime;             // fecha último desmontaje
        int32_t Sb_mnt_count;     //veces montado
        int32_t Sb_magic;      
        int32_t Sb_inode_size;    //sizeof(INODE)
        int32_t Sb_block_size;      // sizeof(FILEBLOCK)=64
        int32_t Sb_first_ino;    //byte del disco donde va el próximo inodo libre
        int32_t Sb_first_blo;    //byte del disco donde va el próximo bloque libre
        int32_t Sb_bm_inode_start; // byte inicio bitmap de inodos
        int32_t Sb_bm_block_start;  // byte inicio bitmap de bloques
        int32_t Sb_inode_start;     // byte inicio tabla de inodos
        int32_t Sb_block_start; 

        //Serialización
        bool Serialize(const std::string& path, long long offset, std::string& errMsg);
        bool Deserialize(const std::string& path, long long offset, std::string& errMsg);

        //Bitmaps
        bool Create_Bit_Maps(const std::string& path, std::string& errMsg);
        bool Update_Inode_Bitmap(const std::string& path, std::string& errMsg);
        bool Update_Block_Bitmap(const std::string& path, std::string& errMsg);

        // Creación de raíz y users.txt
        bool Create_UsersTXT(const std::string& path, std::string& errMsg);
    };
#pragma pack(pop)
} 
#endif