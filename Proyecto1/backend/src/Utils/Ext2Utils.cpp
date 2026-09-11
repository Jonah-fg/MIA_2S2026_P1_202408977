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

    int AsignarBloqueLibre(const string& diskPath, const Estructuras::SUPERBLOCK& sb, string& errMsg) {
        // Leer el bitmap de bloques (arreglo de char '0' y '1')
        int bmSize = sb.Sb_free_blocks_count;
        vector<char> bitmap(bmSize);
        ifstream file(diskPath, ios::binary);
        if (!file.is_open()) {
            errMsg = "No se pudo abrir el disco para leer bitmap de bloques";
            return -1;
        }
        file.seekg(sb.Sb_bm_block_start, ios::beg);
        file.read(bitmap.data(), bmSize);
        if (!file) {
            errMsg = "Error al leer bitmap de bloques";
            return -1;
        }
        file.close();

        // Buscar el primer byte con '0'
        for (int i = 0; i < bmSize; ++i) {
            if (bitmap[i] == '0') {
                // Marcarlo como '1'
                bitmap[i] = '1';
                // Escribir de vuelta
                fstream outFile(diskPath, ios::binary | ios::in | ios::out);
                if (!outFile.is_open()) {
                    errMsg = "No se pudo abrir el disco para escribir bitmap";
                    return -1;
                }
                outFile.seekp(sb.Sb_bm_block_start + i, ios::beg);
                outFile.write(&bitmap[i], 1);
                if (!outFile) {
                    errMsg = "Error al escribir bitmap de bloques";
                    return -1;
                }
                outFile.close();
                return i; // número de bloque
            }
        }
        errMsg = "No hay bloques libres";
        return -1;
    }

    bool EscribirArchivo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, Estructuras::INODE& inode, const string& contenido, string& errMsg) {
        int totalBytes = contenido.size();
        int blockSize = sb.Sb_block_size; // 64
        int numBloquesNecesarios = (totalBytes + blockSize - 1) / blockSize;

        // Verificar que no exceda los 12 bloques directos
        if (numBloquesNecesarios > 12) {
            errMsg = "Archivo demasiado grande (máx 12 bloques = " + to_string(12 * blockSize) + " bytes)";
            return false;
        }

        // Asegurar que el inodo tenga suficientes bloques asignados
        for (int i = 0; i < numBloquesNecesarios; ++i) {
            if (inode.I_block[i]== -1) {
                int nuevoBloque =AsignarBloqueLibre(diskPath, sb, errMsg);
                if (nuevoBloque ==-1) {
                    return false;
                }
                inode.I_block[i] = nuevoBloque;
            }
        }

        // Escribir el contenido en los bloques
        int bytesEscritos = 0;
        for (int i = 0; i < numBloquesNecesarios; ++i) {
            int blockNum = inode.I_block[i];
            long long blockOffset = sb.Sb_block_start + (blockNum * blockSize);
            int bytesRestantes = totalBytes - bytesEscritos;
            int bytesEnEsteBloque = (bytesRestantes > blockSize) ? blockSize : bytesRestantes;

            Estructuras::FILEBLOCK fileBlock;
            memset(&fileBlock, 0, sizeof(fileBlock));
            memcpy(fileBlock.B_content, contenido.data() + bytesEscritos, bytesEnEsteBloque);
            if (!fileBlock.Serialize(diskPath, blockOffset, errMsg)) {
                return false;
            }
            bytesEscritos += bytesEnEsteBloque;
        }

        // Actualizar inodo
        inode.I_size = totalBytes;
        inode.I_mtime = static_cast<float>(time(nullptr));

        // Escribir inodo actualizado (asumimos que es el inodo 1, pero puedes pasarlo como parámetro)
        if (!EscribirInodo(diskPath, sb, 1, inode, errMsg)) {
            return false;
        }

        errMsg.clear();
        return true;
    }
    
    int BuscarInodoLibre(const string& diskPath, const Estructuras::SUPERBLOCK& sb, string& errMsg) {
        // Leer la tabla de inodos completa (todos los inodos)
        int numInodos = sb.Sb_inodes_count;
        vector<Estructuras::INODE> inodos(numInodos);
        ifstream file(diskPath, ios::binary);
        if (!file.is_open()) {
            errMsg = "No se pudo abrir el disco para leer tabla de inodos";
            return -1;
        }
        file.seekg(sb.Sb_inode_start, ios::beg);
        file.read(reinterpret_cast<char*>(inodos.data()), numInodos * sizeof(Estructuras::INODE));
        if (!file) {
            errMsg = "Error al leer tabla de inodos";
            return -1;
        }
        file.close();

        // Buscar el primer inodo que esté libre (I_type[0] == 0 y I_size == 0)
        for (int i = 0; i < numInodos; ++i) {
            if (inodos[i].I_type[0] == 0 && inodos[i].I_size == 0) {
                // También verificar que no tenga bloques asignados (opcional)
                bool tieneBloque = false;
                for (int j = 0; j < 15; ++j) {
                    if (inodos[i].I_block[j] != -1) {
                        tieneBloque = true;
                        break;
                    }
                }
                if(!tieneBloque) {
                    return i;
                }
            }
        }
        errMsg="No hay inodos libres";
        return -1;
    }
}