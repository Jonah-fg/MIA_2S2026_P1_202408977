#include "Mkdir.h"
#include "../../Global/Sesion.h"
#include "../../Global/MountedPartitions.h"
#include <regex>
#include "../../Utils/Ext2Utils.h"
#include "../../Estructuras/Str_Folderblock/FOLDERBLOCK.h"
#include <sstream>
#include <cstring>
using namespace std;

namespace Comandos{
    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i= 0; i<tokens.size(); ++i) {
            if (i >0) {
                result += " ";
            }
            result += tokens[i];
        }
        return result;
    }

    static vector<string> splitPath(const string& path){
        vector<string> partes;
        stringstream ss(path);
        string item;
        while (getline(ss, item, '/')) {
            if (!item.empty()) partes.push_back(item);
        }
        return partes;
    }

    static int buscarInodoPadre(const string& diskPath, const Estructuras::SUPERBLOCK& sb, const string& ruta, string& nombreCarpeta, string& errMsg) {
        vector<string> partes= splitPath(ruta);
        if (partes.empty()){
            errMsg="Ruta vacía";
            return -1;
        }
        nombreCarpeta = partes.back();
        partes.pop_back();

        int inodoActual = 0;
        Estructuras::INODE inode;
        if (!Ext2Utils::LeerInodo(diskPath, sb, inodoActual, inode, errMsg)) {
            return -1;
        }

        for (const string& carpeta : partes) {
            if (inode.I_block[0]== -1) {
                errMsg = "Carpeta no tiene bloques";
                return -1;
            }
            Estructuras::FOLDERBLOCK folderBlock;
            long long blockOffset=sb.Sb_block_start +(inode.I_block[0] * sb.Sb_block_size);
            if (!folderBlock.Deserialize(diskPath, blockOffset, errMsg)){
                return -1;
            }

            bool encontrado= false;
            for (int i =0; i<4; ++i) {
                string nombre= string(folderBlock.B_content[i].B_name);
                nombre =nombre.c_str();
                if (nombre==carpeta){
                    inodoActual = folderBlock.B_content[i].B_inodo;
                    if (!Ext2Utils::LeerInodo(diskPath, sb, inodoActual, inode, errMsg)) 
                        return -1;

                    encontrado= true;
                    break;
                }
            }
            if (!encontrado){
                errMsg = "Carpeta '"+ carpeta +"' no encontrada";
                return -1;
            }
        }
        return inodoActual;
    }

    CommandResult Mkdir_Command(const vector<string>& tokens){
        if (!Global::sesionActual.activa) {
            return{false, "ERROR: No hay sesión activa"};
        }
        string path;
        bool recursive = false;
        (void)recursive;
        string atributos =joinTokens(tokens);
        static const regex lexic(R"(-path="[^"]+"|-path=[^\s]+|-p)", regex::icase);

        vector<string> found;
        auto begin= sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end=sregex_iterator();
        for (auto it =begin; it != end; ++it)
             found.push_back(it->str());

        if (found.size() != tokens.size()){
            for (const auto& token : tokens) {
                if(!regex_search(token, lexic)) {
                    return {false, "ERROR: Parámetro no reconocido: " +token + " en MKDIR"};
                }
            }
        }

        for (const auto& fun : found){
            if (fun =="-p") {
                recursive=true;
                continue;
            }
            size_t eqPos= fun.find('=');
            if (eqPos==string::npos) {
                return {false, "ERROR: formato inválido: " + fun};
            }
            string key= toLowerStr(fun.substr(0, eqPos));
            string value= fun.substr(eqPos + 1);
            if (!value.empty() && value.front() == '"' && value.back() == '"')
                value= value.substr(1, value.size() - 2);

            if (key == "-path") {
                if(value.empty()){
                    return {false, "ERROR: path vacío"};
                }
                path=value;
            } 
            else{
                return {false, "ERROR: parámetro desconocido: " + key};
            }
        }

        if (path.empty()){
            return {false, "ERROR: falta -path"};
        }

        string diskPath = Global::sesionActual.diskPath;
        string id = Global::sesionActual.idParticion;
        Estructuras::PARTITION mountedPart;
        string errMsg;
        if (!Global::GetMountedPartition(id, mountedPart, diskPath, errMsg)) {
            return {false, "ERROR: " + errMsg};
        }

        Estructuras::SUPERBLOCK sb;
        if (!Ext2Utils::LeerSuperbloque(diskPath, mountedPart.Partition_start, sb, errMsg)) {
            return {false,"ERROR: No se pudo ler supebloque: "+ errMsg};
        }

        string nombreCarpeta;
        int inodoPadre =buscarInodoPadre(diskPath, sb, path, nombreCarpeta, errMsg);
        if (inodoPadre == -1) {
            return {false, "ERROR: Ruta no existe: " + errMsg};
        }

        Estructuras::INODE inodePadre;
        if (!Ext2Utils::LeerInodo(diskPath, sb, inodoPadre,inodePadre, errMsg)) {
            return {false, "ERROR: No se pudo leer inodo padre: " + errMsg};
        }

        Estructuras::FOLDERBLOCK folderBlock;
        long long blockOffset=sb.Sb_block_start + (inodePadre.I_block[0] * sb.Sb_block_size);
        if (!folderBlock.Deserialize(diskPath, blockOffset, errMsg)){
            return {false, "ERROR: No se pudo leer bloque de carpeta padre: " + errMsg};
        }

        // Verificacion que la carpeta no exista
        for (int i= 0; i<4; ++i) {
            string nombre = string(folderBlock.B_content[i].B_name);
            nombre = nombre.c_str();
            if(nombre == nombreCarpeta) {
                return {false, "ERROR: La carpeta '" +nombreCarpeta+ "' ya existe"};
            }
        }

        //Buscador slot libre
        int slotLibre =-1;
        for (int i=0; i < 4; ++i) {
            string nombre = string(folderBlock.B_content[i].B_name);
            nombre = nombre.c_str();
            if (nombre.empty() || nombre== "-" || folderBlock.B_content[i].B_inodo == -1) {
                slotLibre =i;
                break;
            }
        }
        if(slotLibre == -1) {
            return {false, "ERROR: Carpeta padre llena"};
        }

        //Buscador inodo libre para nueva carpeta
        int nuevoInodo=Ext2Utils::BuscarInodoLibre(diskPath, sb, errMsg);
        if (nuevoInodo ==-1) {
            return {false, "ERROR: No hay inodos libres: " + errMsg};
        }

        //Creacion inodo de carpeta
        Estructuras::INODE newInode;
        memset(&newInode, 0, sizeof(newInode));
        newInode.I_uid= Global::sesionActual.uid;
        newInode.I_gid = Global::sesionActual.gid;
        newInode.I_size =0;
        newInode.I_atime =static_cast<float>(time(nullptr));
        newInode.I_ctime = static_cast<float>(time(nullptr));
        newInode.I_mtime=static_cast<float>(time(nullptr));
        newInode.I_type[0] = '0';
        newInode.I_perm[0]= '6';
        newInode.I_perm[1] = '6';
        newInode.I_perm[2] ='4';

        //Asignacion
        int bloque=Ext2Utils::AsignarBloqueLibre(diskPath, sb, errMsg);
        if (bloque ==-1){
            return {false, "ERROR: No hay bloques libres: " +errMsg};
        }
        newInode.I_block[0] = bloque;
        for (int i = 1; i< 15; ++i){
            newInode.I_block[i] = -1;
        }

        Estructuras::FOLDERBLOCK newFolderBlock;
        memset(&newFolderBlock, 0, sizeof(newFolderBlock));
        newFolderBlock.B_content[0].B_name[0] = '.';
        newFolderBlock.B_content[0].B_inodo = nuevoInodo;
        newFolderBlock.B_content[1].B_name[0]= '.';
        newFolderBlock.B_content[1].B_name[1]= '.';
        newFolderBlock.B_content[1].B_inodo = inodoPadre;
        newFolderBlock.B_content[2].B_name[0] = '-';
        newFolderBlock.B_content[2].B_inodo = -1;
        newFolderBlock.B_content[3].B_name[0]='-';
        newFolderBlock.B_content[3].B_inodo=-1;

        //Escritura del bloqu
        long long newBlockOffset= sb.Sb_block_start + (bloque * sb.Sb_block_size);
        if (!newFolderBlock.Serialize(diskPath, newBlockOffset, errMsg)) {
            return {false, "ERROR: No se pudo escribir bloque de carpeta: "+errMsg};
        }
        //Escritura inodo
        if (!Ext2Utils::EscribirInodo(diskPath, sb, nuevoInodo, newInode, errMsg)) {
            return {false, "ERROR: No se pudo escribir inodo de carpeta: " + errMsg};
        }

        //Marca inodo como usado
        if (!Ext2Utils::MarcarInodoUsado(diskPath, sb, nuevoInodo, errMsg)){
            return {false, "ERROR: No se pudo marcar inodo como usado: " +errMsg};
        }

        //Actualizacio superbloque
        sb.Sb_inodes_count++;
        sb.Sb_free_inodes_count--;
        sb.Sb_first_ino+= sb.Sb_inode_size;
        sb.Sb_blocks_count++;
        sb.Sb_free_blocks_count--;
        sb.Sb_first_blo += sb.Sb_block_size;
        if(!sb.Serialize(diskPath, mountedPart.Partition_start, errMsg)) {
            return {false, "ERROR: No se pudo actualizar superbloque: " +errMsg};
        }

        folderBlock.B_content[slotLibre].B_inodo = nuevoInodo;
        strncpy(folderBlock.B_content[slotLibre].B_name, nombreCarpeta.c_str(), 12);
        if(!folderBlock.Serialize(diskPath, blockOffset, errMsg)) {
            return {false,"ERROR: No se pudo actualizar carpeta padre: "+ errMsg};
        }

        // Actualizacion fecha de modificación de la carpeta padre
        inodePadre.I_mtime =static_cast<float>(time(nullptr));
        if (!Ext2Utils::EscribirInodo(diskPath, sb, inodoPadre, inodePadre, errMsg)) {
            return {false, "ERROR: No se pudo actualizar inodo padre: " +errMsg};
        }
        return {true, "MKDIR: Carpeta '" + nombreCarpeta +"' creada (inodo " + to_string(nuevoInodo) + ")"};
    }
} 