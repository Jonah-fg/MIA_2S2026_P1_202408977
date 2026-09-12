#include "Cat.h"
#include "../../Global/Sesion.h"
#include "../../Global/MountedPartitions.h"
#include "../../Utils/Ext2Utils.h"
#include "../../Estructuras/Str_Folderblock/FOLDERBLOCK.h"
#include <regex>
#include <sstream>
using namespace std;

namespace Comandos{

    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i =0; i<tokens.size(); ++i) {
            if (i > 0) 
            result += " ";

            result +=tokens[i];
        }
        return result;
    }

    static vector<string> splitPath(const string& path) {
        vector<string> partes;
        stringstream ss(path);
        string item;
        while (getline(ss, item, '/')){
            if (!item.empty()) partes.push_back(item);
        }
        return partes;
    }

    // Obtiene el inodo de un archivo a partir de su ruta absoluta
    static int obtenerInodoArchivo(const string& diskPath, const Estructuras::SUPERBLOCK& sb, const string& ruta, string& errMsg) {
        vector<string> partes = splitPath(ruta);
        if (partes.empty()){
            errMsg ="Ruta vacía";
            return -1;
        }

        int inodoActual =0;
        Estructuras::INODE inode;
        if (!Ext2Utils::LeerInodo(diskPath, sb, inodoActual, inode, errMsg)) return -1;

        for (const string& componente : partes){
            if (inode.I_block[0]== -1){
                errMsg= "Carpeta no tiene bloques";
                return -1;
            }
            Estructuras::FOLDERBLOCK folderBlock;
            long long blockOffset= sb.Sb_block_start + (inode.I_block[0] * sb.Sb_block_size);
            if (!folderBlock.Deserialize(diskPath, blockOffset, errMsg)) 
                return -1;

            bool encontrado =false;
            for (int i=0; i <4; ++i) {
                string nombre = string(folderBlock.B_content[i].B_name);
                nombre =nombre.c_str();
                if(nombre==componente){
                    inodoActual = folderBlock.B_content[i].B_inodo;
                    if (!Ext2Utils::LeerInodo(diskPath, sb, inodoActual, inode, errMsg)) {
                        return -1;
                    }
                    encontrado= true;
                    break;
                }
            }
            if (!encontrado){
                errMsg ="Componente '" + componente+ "' no encontrado";
                return -1;
            }
        }
        if (inode.I_type[0]!= '1'){
            errMsg ="No es un archivo";
            return -1;
        }
        return inodoActual;
    }

    CommandResult Cat_Command(const vector<string>& tokens){
        if (!Global::sesionActual.activa){
            return {false, "ERROR: No hay sesión activa"};
        }

        //Parseo -file1, -file2...
        vector<string> archivos;
        string atributos =joinTokens(tokens);
        static const regex lexic( R"(-file\d+="[^"]+"|-file\d+=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin= sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end =sregex_iterator();
        for (auto it=begin; it != end; ++it) {
            found.push_back(it->str());
        }

        if (found.size()!= tokens.size()) {
            for (const auto& token : tokens) {
                if (!regex_search(token, lexic)){
                    return {false, "ERROR: Parámetro no reconocido: " + token + " en CAT"};
                }
            }
        }

        for (const auto& fun : found) {
            size_t eqPos = fun.find('=');
            if (eqPos == string::npos) {
                return {false, "ERROR: formato inválido: " + fun};
            }
            string key = toLowerStr(fun.substr(0, eqPos));
            string value =fun.substr(eqPos + 1);
            if (!value.empty() && value.front() == '"' && value.back() == '"'){
                value=value.substr(1, value.size()- 2);
            }

            if (key.find("-file")== 0) {
                if(value.empty())
                    return {false, "ERROR: ruta de archio vacía"};
                
                archivos.push_back(value);
            } 
            else{
                return {false, "ERROR: parámetro desconocido: " + key};
            }
        }

        if (archivos.empty()) {
            return {false, "ERROR: No se especificaron archivos"};
        }

        string diskPath = Global::sesionActual.diskPath;
        string id =Global::sesionActual.idParticion;
        Estructuras::PARTITION mountedPart;
        string errMsg;
        if (!Global::GetMountedPartition(id, mountedPart, diskPath, errMsg)) {
            return {false, "ERROR: "+ errMsg};
        }

        Estructuras::SUPERBLOCK sb;
        if (!Ext2Utils::LeerSuperbloque(diskPath, mountedPart.Partition_start, sb, errMsg)) {
            return {false, "ERROR: No se pudo leer superbloque: " + errMsg};
        }

        string output;
        for (const string& ruta : archivos) {
            int inodoNum=obtenerInodoArchivo(diskPath, sb, ruta, errMsg);
            if(inodoNum== -1) {
                return {false, "ERROR: No se pudo encontrar archivo '" +ruta + "': " +errMsg};
            }

            Estructuras::INODE inode;
            if (!Ext2Utils::LeerInodo(diskPath, sb, inodoNum, inode, errMsg)) {
                return{false, "ERROR: No se pudo leer inodo de '" + ruta+ "': " +errMsg};
            }

            string contenido;
            if (!Ext2Utils::LeerArchivo(diskPath, sb, inode, contenido, errMsg)){
                return {false, "ERROR: No se pudo leer contenido de '" + ruta+ "': " +errMsg};
            }

            output += contenido;
            if (archivos.size() > 1) output += "\n";
        }
        return {true, output};
    }
}