#include "Mkfs.h"
#include "../../Global/MountedPartitions.h"
#include "../../Estructuras/Str_Superblock/SUPERBLOCK.h"
#include "../../Estructuras/Str_Inode/INODE.h"
#include "../../Estructuras/Str_Fileblock/FILEBLOCK.h"
#include "../../Estructuras/Str_Folderblock/FOLDERBLOCK.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cmath>
#include <cstring>
#include <ctime>
using namespace std;

namespace Comandos{

    //Helpers
    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens){
        string result;
        for (size_t i=0; i<tokens.size(); ++i) {
            if (i > 0) result +=" ";
            result +=tokens[i];
        }
        return result;
    }

    // Calcula "n" (número de inodos) que caben en la partición
    static int32_t calculate_N(const Estructuras::PARTITION& part) {
        long long numerador = static_cast<long long>(part.Partition_size) - static_cast<long long>(sizeof(Estructuras::SUPERBLOCK));
        long long denominador = 4+static_cast<long long>(sizeof(Estructuras::INODE)) + (3 * static_cast<long long>(sizeof(Estructuras::FILEBLOCK)));
        double n =floor(static_cast<double>(numerador) / static_cast<double>(denominador));
        return static_cast<int32_t>(n);
    }

    //Se arma el superbloque con los offsets calculados
    static Estructuras::SUPERBLOCK Create_SuperBlock(const Estructuras::PARTITION& part, int32_t n_Value) {
        int32_t bm_inode_start =part.Partition_start + static_cast<int32_t>(sizeof(Estructuras::SUPERBLOCK));
        int32_t bm_block_start= bm_inode_start + n_Value;
        int32_t inode_start=bm_block_start + (3 * n_Value);
        int32_t block_start =inode_start + (static_cast<int32_t>(sizeof(Estructuras::INODE)) * n_Value);

        Estructuras::SUPERBLOCK sb;
        memset(&sb, 0, sizeof(sb));
        sb.Sb_filesystem_type =2; 
        sb.Sb_inodes_count = 0;
        sb.Sb_blocks_count=0;
        sb.Sb_free_inodes_count = n_Value;
        sb.Sb_free_blocks_count = n_Value *3;
        sb.Sb_mtime = static_cast<float>(time(nullptr));
        sb.Sb_umtime = static_cast<float>(time(nullptr));
        sb.Sb_mnt_count = 1;
        sb.Sb_magic=0xEF53;
        sb.Sb_inode_size = static_cast<int32_t>(sizeof(Estructuras::INODE));
        sb.Sb_block_size = static_cast<int32_t>(sizeof(Estructuras::FILEBLOCK));
        sb.Sb_first_ino =inode_start;
        sb.Sb_first_blo= block_start;
        sb.Sb_bm_inode_start = bm_inode_start;
        sb.Sb_bm_block_start=bm_block_start;
        sb.Sb_inode_start=inode_start;
        sb.Sb_block_start =block_start;
        return sb;
    }

    //planea todo el formateo
    static bool Create_MKFS(const MKFS& mkfs, string& errMsg){
        //Obtener la partición montada y el path del disco
        Estructuras::PARTITION mountedPart;
        string diskPath;
        if (!Global::GetMountedPartition(mkfs.Id, mountedPart, diskPath, errMsg)) {
            return false;
        }

        //Calculo de n
        int32_t n_Value =calculate_N(mountedPart);
        if (n_Value <= 0){
            errMsg ="La partición es demsiado pequeña para EXT2 (n=" + to_string(n_Value)+")";
            return false;
        }

        //Creacion superbloque en memoria
        Estructuras::SUPERBLOCK sb=Create_SuperBlock(mountedPart, n_Value);

        //Creacion bitmaps 
        if (!sb.Create_Bit_Maps(diskPath, errMsg)) {
            return false;
        }

        //Creacion raíz y users.txt
        if (!sb.Create_UsersTXT(diskPath, errMsg)){
            return false;
        }

        //Guardado superbloque en el disco
        if (!sb.Serialize(diskPath, mountedPart.Partition_start, errMsg)) {
            errMsg = "Error al guardar superbloque: "+ errMsg;
            return false;
        }
        return true;
    }

    //Punto de entrada del comando
    CommandResult Mkfs_Command(const vector<string>& tokens) {
        MKFS mkfs;
        string atributos= joinTokens(tokens);
        static const regex lexic(R"(-id=[^\s]+|-type=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin =sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end= sregex_iterator();
        for (auto it=begin; it != end; ++it){
            found.push_back(it->str());
        }

        for (const auto& fun : found){
            size_t eqPos= fun.find('=');
            if (eqPos== string::npos){
                return {false, "ERROR: formato de parametros invalido: " + fun};
            }

            string key=toLowerStr(fun.substr(0, eqPos));
            string value = fun.substr(eqPos + 1);
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"'){
                value = value.substr(1, value.size()-2);
            }

            if (key =="-id") {
                if (value.empty()) return {false, "ERROR: id vacío"};
                mkfs.Id = value;
            } 
            else if (key == "-type"){
                string v= toLowerStr(value);
                if (v != "full")
                    return {false, "ERROR: type solo 'full' soportado"};

                mkfs.Type =v;
            } 
            else {
                return{false, "ERROR: parámetro desconocido: " + key};
            }
        }
        if (mkfs.Id.empty()) {
            return {false, "ERROR: falta -id"};
        }

        string errMsg;
        if (!Create_MKFS(mkfs, errMsg))
            return {false, errMsg};

        return {true, "COMANDO MKFS: partición formateada con xito"};
    }

} 