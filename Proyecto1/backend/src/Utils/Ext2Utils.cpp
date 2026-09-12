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
        // El bitmap debe cubrir el total de bloques del sistema, no solo la cantidad de libres.
        int bmSize = sb.Sb_blocks_count+ sb.Sb_free_blocks_count;
        vector<char> bitmap(static_cast<size_t>(bmSize), '0');
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

        for (int i = 0; i < bmSize; ++i) {
            if (bitmap[i] == '0') {
                bitmap[i] = '1';
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
                return i;
            }
        }
        errMsg = "No hay bloques libres";
        return -1;
    }

    bool EscribirArchivo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, int inodoNum, Estructuras::INODE& inode, const string& contenido, string& errMsg) {
        int totalBytes =contenido.size();
        int blockSize=sb.Sb_block_size; //64
        int numBloquesNecesarios =(totalBytes + blockSize - 1) / blockSize;

        //Verificacion que no exceda los 12 bloques directos
        if (numBloquesNecesarios >12){
            errMsg = "Archivo demasiado grande (máx 12 bloques = " + to_string(12 * blockSize) + " bytes)";
            return false;
        }
        // Asegurar que el inodo tenga suficientes bloques asignados
        for (int i = 0; i<numBloquesNecesarios; ++i) {
            if (inode.I_block[i]== -1 || inode.I_block[i] == 0) {
                int nuevoBloque =AsignarBloqueLibre(diskPath, sb, errMsg);
                if (nuevoBloque ==-1) {
                    return false;
                }
                inode.I_block[i] = nuevoBloque;
            }
        }

        // Escribir el contenido en los bloques
        int bytesEscritos = 0;
        for (int i = 0; i< numBloquesNecesarios; ++i) {
            int blockNum = inode.I_block[i];
            long long blockOffset = sb.Sb_block_start + (blockNum * blockSize);
            int bytesRestantes =totalBytes - bytesEscritos;
            int bytesEnEsteBloque=(bytesRestantes > blockSize) ? blockSize : bytesRestantes;

            Estructuras::FILEBLOCK fileBlock;
            memset(&fileBlock, 0, sizeof(fileBlock));
            memcpy(fileBlock.B_content, contenido.data() + bytesEscritos, bytesEnEsteBloque);
            if (!fileBlock.Serialize(diskPath, blockOffset, errMsg)) {
                return false;
            }
            bytesEscritos += bytesEnEsteBloque;
        }

        // Actualizar inodo del archivo correcto
        inode.I_size = totalBytes;
        inode.I_mtime = static_cast<float>(time(nullptr));

        if(!EscribirInodo(diskPath, sb, inodoNum, inode, errMsg)) {
            return false;
        }

        errMsg.clear();
        return true;
    }
    
    int BuscarInodoLibre(const string& diskPath, const Estructuras::SUPERBLOCK& sb, string& errMsg) {
        // El bitmap y la tabla de inodos deben abarcar el total de inodos del sistema.
        int totalInodos = sb.Sb_inodes_count + sb.Sb_free_inodes_count;
        vector<Estructuras::INODE> inodos(static_cast<size_t>(totalInodos));
        ifstream file(diskPath, ios::binary);
        if (!file.is_open()) {
            errMsg = "No se pudo abrir el disco para leer tabla de inodos";
            return -1;
        }
        file.seekg(sb.Sb_inode_start, ios::beg);
        file.read(reinterpret_cast<char*>(inodos.data()), totalInodos * sizeof(Estructuras::INODE));
        if (!file) {
            errMsg = "Error al leer tabla de inodos";
            return -1;
        }
        file.close();

        for (int i = 0; i < totalInodos; ++i) {
            const Estructuras::INODE& inode = inodos[i];

            // Un inode libre debe estar totalmente en cero y sin bloques reales asignados.
            // Importante: los inodos recién creados se inicializan con 0 en I_block[], no con -1.
            if (inode.I_type[0] == '\0' && inode.I_size == 0) {
                bool tieneBloqueReal = false;
                for (int j = 0; j < 15; ++j) {
                    if (inode.I_block[j] != 0 && inode.I_block[j] != -1) {
                        tieneBloqueReal = true;
                        break;
                    }
                }
                if (!tieneBloqueReal) {
                    return i;
                }
            }
        }
        errMsg = "No hay inodos libres";
        return -1;
    }

    bool MarcarInodoUsado(const string& diskPath, const Estructuras::SUPERBLOCK& sb, int inodoNum, string& errMsg) {
        fstream file(diskPath, ios::binary | ios::in | ios::out);
        if(!file.is_open()) {
            errMsg= "No se pudo abrir el disco para marcar inodo usado";
            return false;
        }
        long long pos = sb.Sb_bm_inode_start+inodoNum;
        file.seekp(pos, ios::beg);
        char bit ='1';
        file.write(&bit, 1);
        if (!file){
            errMsg = "Error al escribir en el bitmap de inodos";
            file.close();
            return false;
        }
        file.close();
        errMsg.clear();
        return true;
    }

    bool MarcarBloqueUsado(const string& diskPath, const Estructuras::SUPERBLOCK& sb,int bloqueNum, string& errMsg) {
        fstream file(diskPath, ios::binary | ios::in | ios::out);
        if (!file.is_open()){
            errMsg="No se pudo abrir el disco para marar bloque usado";
            return false;
        }
        long long pos =sb.Sb_bm_block_start + bloqueNum;
        file.seekp(pos, ios::beg);
        char bit ='1';
        file.write(&bit, 1);
        if (!file){
            errMsg= "Error al escribir en el bitmap de bloques";
            file.close();
            return false;
        }
        file.close();
        errMsg.clear();
        return true;
    }
}