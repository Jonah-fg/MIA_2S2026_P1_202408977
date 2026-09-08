#include "Ext2Utils.h"
#include "../Estructuras/Str_Fileblock/FILEBLOCK.h"
#include "../Estructuras/Str_Folderblock/FOLDERBLOCK.h"
#include <fstream>
#include <cstring>
#include <ctime>
using namespace std;
namespace Ext2Utils{

    bool LeerSuperbloque(const string& diskPath, int partitionStart, Estructuras::SUPERBLOCK& sb, string& errMsg) {
        return sb.Deserialize(diskPath, partitionStart, errMsg);
    }

    bool LeerInodo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, int inodoNum, Estructuras::INODE& inode, string& errMsg) {
        long long offset = sb.Sb_inode_start + (inodoNum * sb.Sb_inode_size);
        return inode.Deserialize(diskPath, offset, errMsg);
    }

    bool EscribirInodo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, int inodoNum, const Estructuras::INODE& inode, string& errMsg) {
        long long offset= sb.Sb_inode_start + (inodoNum*sb.Sb_inode_size);
        Estructuras::INODE temp= inode;
        return temp.Serialize(diskPath, offset, errMsg);
    }

    bool LeerArchivo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, const Estructuras::INODE& inode, string& contenido, string& errMsg) {
        contenido.clear();
        //Solo soporta bloques directos al momento
        int totalBytes= inode.I_size;
        int bytesLeidos=0;

        for (int i = 0; i <12; ++i) {//12 bloques directos
            if (inode.I_block[i] == -1){
                break;
            }

            int blockNum=inode.I_block[i];
            long long blockOffset = sb.Sb_block_start + (blockNum* sb.Sb_block_size);

            Estructuras::FILEBLOCK fileBlock;
            if (!fileBlock.Deserialize(diskPath, blockOffset, errMsg)) {
                return false;
            }

            int bytesPorBloque = sb.Sb_block_size;
            if (bytesLeidos + bytesPorBloque > totalBytes) {
                bytesPorBloque = totalBytes - bytesLeidos;
            }
            contenido.append(fileBlock.B_content, bytesPorBloque);
            bytesLeidos += bytesPorBloque;
            if (bytesLeidos >= totalBytes){
                break;
            }
        }
        if (bytesLeidos != totalBytes){
            errMsg = "No se pudo leer todo el archivo (tamaño esperado " + to_string(totalBytes) + ", leído " + to_string(bytesLeidos) + ")";
            return false;
        }
        return true;
    }

    bool EscribirArchivo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, Estructuras::INODE& inode, const string& contenido, string& errMsg) {
        if (contenido.size() > sb.Sb_block_size){
            errMsg = "Contenido demasido grande para un solo bloque (máx "+ to_string(sb.Sb_block_size) + " bytes)";
            return false;
        }

        // Asignacion de un bloque si no tiene 
        int blockNum =inode.I_block[0];
        if (blockNum== -1) {
            errMsg = "El inodo no tiene bloque asignado y no implementams asignación dinámica aún";
            return false;
        }
        long long blockOffset= sb.Sb_block_start + (blockNum * sb.Sb_block_size);
        Estructuras::FILEBLOCK fileBlock;
        memset(&fileBlock, 0, sizeof(fileBlock));
        memcpy(fileBlock.B_content, contenido.data(), contenido.size());

        if(!fileBlock.Serialize(diskPath, blockOffset, errMsg)) {
            return false;
        }

        // Actualizacion tamaño e inodo
        inode.I_size =contenido.size();
        inode.I_mtime= static_cast<float>(time(nullptr));
        if (!EscribirInodo(diskPath, sb, 1, inode, errMsg)){
            return false;  //users.txt es inodo 1 creo
        } 
        return true;
    }
} 