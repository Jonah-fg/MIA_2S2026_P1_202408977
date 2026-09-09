#ifndef EXT2_UTILS_H
#define EXT2_UTILS_H
#include <string>
#include <vector>
#include "../Estructuras/Str_Superblock/SUPERBLOCK.h"
#include "../Estructuras/Str_Inode/INODE.h"
namespace Ext2Utils {
    bool LeerSuperbloque(const std::string& diskPath, int partitionStart, Estructuras::SUPERBLOCK& sb, std::string& errMsg);
    // Lee un inodo dado su número 
    bool LeerInodo(const std::string& diskPath, const Estructuras::SUPERBLOCK& sb, int inodoNum, Estructuras::INODE& inode, std::string& errMsg);
    //inodo en tabla
    bool EscribirInodo(const std::string& diskPath, const Estructuras::SUPERBLOCK& sb, int inodoNum, const Estructuras::INODE& inode, std::string& errMsg);
    bool LeerArchivo(const std::string& diskPath, const Estructuras::SUPERBLOCK& sb, const Estructuras::INODE& inode, std::string& contenido, std::string& errMsg);
    bool EscribirArchivo(const std::string& diskPath, const Estructuras::SUPERBLOCK& sb, Estructuras::INODE& inode, const std::string& contenido, std::string& errMsg);
    //Asigna un bloque libre 
    int AsignarBloqueLibre(const std::string& diskPath, const Estructuras::SUPERBLOCK& sb, std::string& errMsg);
} 
#endif