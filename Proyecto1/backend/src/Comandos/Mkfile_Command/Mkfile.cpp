#include "Mkfile.h"
#include "../../Global/Sesion.h"
#include "../../Global/MountedPartitions.h"
#include "../../Utils/Ext2Utils.h"
#include "../../Utils/UsersUtils.h"
#include "../../Estructuras/Str_Folderblock/FOLDERBLOCK.h"
#include <iostream>
#include <regex>
#include "../../Estructuras/Str_Fileblock/FILEBLOCK.h"
#include <sstream>
#include <fstream>
#include <cstring>
using namespace std;

namespace Comandos {
    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i = 0; i<tokens.size(); ++i) {
            if (i>0) 
            result += " ";

            result +=tokens[i];
        }
        return result;
    }

    //Función para dividir una ruta en componentes
    static vector<string> splitPath(const string& path) {
        vector<string> partes;
        stringstream ss(path);
        string item;
        while (getline(ss, item, '/')){
            if(!item.empty()) partes.push_back(item);
        }
        return partes;
    }

    //Busca un inodo a partir de una ruta absoluta 
    static int buscarInodoPorRuta(const string& diskPath, const Estructuras::SUPERBLOCK& sb, const string& ruta, int& inodoPadre, string& nombreArchivo, string& errMsg) {
        vector<string> partes= splitPath(ruta);
        if (partes.empty()) {
            errMsg ="Ruta vacía";
            return -1;
        }

        nombreArchivo= partes.back();
        partes.pop_back(); //quedan las carpetas padres

        //Empezamos desde el inodo raíz (0)
        int inodoActual=0;
        Estructuras::INODE inode;
        if (!Ext2Utils::LeerInodo(diskPath, sb, inodoActual, inode, errMsg)) {
            return -1;
        }

        //Recorre las carpetas padres
        for (const string& carpeta : partes){
            if (inode.I_block[0]== -1) {
                errMsg = "Capeta no tiene bloques";
                return -1;
            }
            Estructuras::FOLDERBLOCK folderBlock;
            long long blockOffset =sb.Sb_block_start+(inode.I_block[0] * sb.Sb_block_size);
            if (!folderBlock.Deserialize(diskPath, blockOffset, errMsg)) {
                return -1;
            }

            //Busca la entrada con el nombre de la carpeta
            bool encontrado=false;
            for (int i = 0; i <4; ++i) {
                string nombre=string(folderBlock.B_content[i].B_name);
                nombre =nombre.c_str(); //basura
                if (nombre ==carpeta) {
                    inodoActual = folderBlock.B_content[i].B_inodo;
                    if (!Ext2Utils::LeerInodo(diskPath, sb, inodoActual, inode, errMsg)) {
                        return -1;
                    }
                    encontrado=true;
                    break;
                }
            }
            if (!encontrado) {
                errMsg = "Carpeta '" + carpeta + "' no encontrada";
                return -1;
            }
        }
        //inodoActual es el inodo de la carpeta padre
        inodoPadre=inodoActual;
        return inodoActual;
    }

    CommandResult Mkfile_Command(const vector<string>& tokens) {
        if (!Global::sesionActual.activa){
            return {false, "ERROR: No hay sesión activa"};
        }
        // Parseo parámetros: -path, -size, -cont, -r 
        string path;
        bool recursive=false;
        int size =-1;
        string contPath;

        string atributos =joinTokens(tokens);
        static const regex lexic(R"(-path="[^"]+"|-path=[^\s]+|-r|-size=\d+|-cont="[^"]+"|-cont=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin =sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end=sregex_iterator();
        for (auto it = begin; it !=end; ++it){
            found.push_back(it->str());
        }

        //Verificacion que todos los tokens sean reconocidos
        if (found.size() !=tokens.size()) {
            for (const auto& token : tokens){
                if (!regex_search(token, lexic)) {
                    return {false, "ERROR: Parámetro no reconocido: " +token+ " en MKFILE"};
                }
            }
        }

        for (const auto& fun : found) {
            if (fun=="-r"){
                recursive =true;
                continue;
            }
            size_t eqPos = fun.find('=');
            if (eqPos == string::npos && fun != "-r") {
                return {false, "ERROR: formato inválido: " + fun};
            }
            string key =(eqPos == string::npos) ? fun : fun.substr(0, eqPos);
            string value = (eqPos == string::npos) ? "" : fun.substr(eqPos+1);
            if(!value.empty() && value.front() == '"' && value.back()== '"'){
                value = value.substr(1, value.size()- 2);
            }

            string keyLower = toLowerStr(key);
            if (keyLower == "-path") {
                if (value.empty()) {
                    return {false, "ERROR: path vacío"};
                }
                path= value;
            } 
            else if (keyLower== "-size") {
                try {
                    size=stoi(value);
                    if (size < 0){
                       return {false, "ERROR: size debe ser >= 0"}; 
                    }
                } 
                catch (...) {
                    return {false, "ERROR: size debe ser un número"};
                }
            } 
            else if (keyLower =="-cont") {
                if (value.empty()) 
                    return {false, "ERROR: cont vacío"};

                contPath = value;
            } 
            else if(keyLower =="-r") {
            }
            else{
                return{false, "ERROR: parámetro desconocido: " + key};
            }
        }
        if (path.empty()) {
            return {false, "ERROR: falta -path"};
        }

        // Obtencion partición y superbloqueu
        string diskPath= Global::sesionActual.diskPath;
        string id = Global::sesionActual.idParticion;
        Estructuras::PARTITION mountedPart;
        string errMsg;
        if(!Global::GetMountedPartition(id, mountedPart, diskPath, errMsg)) {
            return {false, "ERROR: "+errMsg};
        }

        Estructuras::SUPERBLOCK sb;
        if (!Ext2Utils::LeerSuperbloque(diskPath, mountedPart.Partition_start, sb, errMsg)) {
            return {false, "ERROR: No se pudo leer superloque: " + errMsg};
        }

        //Busca el inodo de la carpeta padre y el nombre del archivo
        int inodoPadre;
        string nombreArchivo;
        int padre=buscarInodoPorRuta(diskPath, sb, path, inodoPadre, nombreArchivo, errMsg);
        if (padre == -1){
            if(recursive){
                return{false, "ERROR: No se implementó creación recursiva de carpetas aún"};
            } 
            else{
                return {false, "ERROR: Ruta no existe: " +errMsg};
            }
        }

        //lectura inodo de la carpeta padre
        Estructuras::INODE inodePadreObj;
        if(!Ext2Utils::LeerInodo(diskPath, sb, inodoPadre, inodePadreObj, errMsg)) {
            return {false, "ERROR: No se pudo leer inodo padre: "+errMsg};
        }

        //Verificacion 
        Estructuras::FOLDERBLOCK folderBlock;
        long long blockOffset =sb.Sb_block_start +(inodePadreObj.I_block[0]*sb.Sb_block_size);
        if (!folderBlock.Deserialize(diskPath, blockOffset, errMsg)) {
            return {false, "ERROR: No se pudo leer bloque de carpeta padre: " + errMsg};
        }
        for (int i=0; i< 4; ++i) {
            string nombre = string(folderBlock.B_content[i].B_name);
            nombre =nombre.c_str();
            if (nombre== nombreArchivo) {
                return {false, "ERROR: El archivo '" + nombreArchivo+ "' ya existe en la carpeta"};
            }
        }

        // Buscar un slot libre 
        int slotLibre=-1;
        for (int i = 0; i<4; ++i) {
            string nombre = string(folderBlock.B_content[i].B_name);
            nombre =nombre.c_str();
            if (nombre.empty() || nombre== "-" || folderBlock.B_content[i].B_inodo == -1) {
                slotLibre=i;
                break;
            }
        }
        if(slotLibre == -1) {
            return {false, "ERROR: Carpeta padre llena (máx 4 entrdas)"};
        }

        //contenido del archivo
        string contenido;
        if(!contPath.empty()){
            ifstream fileCont(contPath);
            if (!fileCont.is_open()) {
                return {false, "ERROR: No se pudo abrir archivo de contenido: " + contPath};
            }
            stringstream buffer;
            buffer << fileCont.rdbuf();
            contenido = buffer.str();
        } 
        else if (size >= 0){
            contenido.reserve(size);
            for (int i =0; i<size; ++i) {
                contenido.push_back('0'+(i % 10));
            }
        } 
        else{
            contenido = "";
        }

        //Asignacion inodo libre
        int nuevoInodo=Ext2Utils::BuscarInodoLibre(diskPath, sb, errMsg);
        if (nuevoInodo ==-1){
            return {false, "ERROR: No hay inodos libres: " +errMsg};
        }

        //Creacion del inodo del archivo
        Estructuras::INODE newInode;
        memset(&newInode, 0, sizeof(newInode));
        newInode.I_uid= Global::sesionActual.uid;
        newInode.I_gid=Global::sesionActual.gid;
        newInode.I_size= contenido.size();
        newInode.I_atime =static_cast<float>(time(nullptr));
        newInode.I_ctime = static_cast<float>(time(nullptr));
        newInode.I_mtime=static_cast<float>(time(nullptr));
        newInode.I_type[0] = '1';
        newInode.I_perm[0]= '6';
        newInode.I_perm[1] = '6';
        newInode.I_perm[2] = '4';
        if (!contenido.empty()){
            for (int i=0; i<15; ++i) {
                newInode.I_block[i] =-1;
            }
            //Se escribe inodo en discoo
            if (!Ext2Utils::EscribirInodo(diskPath, sb, nuevoInodo, newInode, errMsg)) {
                return {false, "ERROR: No se pudo escribir inodo: " + errMsg};
            }
            //Escribierndo el contenido 
            if (!Ext2Utils::EscribirArchivo(diskPath, sb, nuevoInodo, newInode, contenido, errMsg)) {
                return {false, "ERROR: No se pudo escrbir contenido: " +errMsg};
            }
        } 
        else{
            for(int i= 0; i<15; ++i){
                newInode.I_block[i] = -1; 
            }
            if (!Ext2Utils::EscribirInodo(diskPath, sb, nuevoInodo, newInode, errMsg)) {
                return {false, "ERROR: No se pudo escribir inodo: "+ errMsg};
            }
        }

        //Actualizacion bitmap de inodos
        if (!sb.Update_Inode_Bitmap(diskPath, errMsg)){
            return {false, "ERROR: No se pudo actualizar bitmap de inodos: "+ errMsg};
        }
        sb.Sb_inodes_count++;
        sb.Sb_free_inodes_count--;
        sb.Sb_first_ino +=sb.Sb_inode_size;
        // Escritura superbloque actualñizado
        if(!sb.Serialize(diskPath, mountedPart.Partition_start, errMsg)) {
            return {false, "ERROR: No se pudo actualizar superbloque: "+ errMsg};
        }

        folderBlock.B_content[slotLibre].B_inodo= nuevoInodo;
        strncpy(folderBlock.B_content[slotLibre].B_name, nombreArchivo.c_str(), 12);
        if(!folderBlock.Serialize(diskPath, blockOffset, errMsg)){
            return {false, "ERROR: No se pudo actualizar carpeta padre: "+ errMsg};
        }

        inodePadreObj.I_mtime= static_cast<float>(time(nullptr));
        if (!Ext2Utils::EscribirInodo(diskPath, sb, inodoPadre, inodePadreObj, errMsg)) {
            return {false, "ERROR: No se pudo actualizar inodo adre: "+ errMsg};
        }
        return {true, "MKFILE: Archivo '" +nombreArchivo+ "' creado exitosamente (inodo " + to_string(nuevoInodo) + ")"};
    }

} 