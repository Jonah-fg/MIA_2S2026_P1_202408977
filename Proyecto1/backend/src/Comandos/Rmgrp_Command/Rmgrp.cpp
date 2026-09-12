#include "Rmgrp.h"
#include "../../Global/Sesion.h"
#include "../../Global/MountedPartitions.h"
#include "../../Utils/Ext2Utils.h"
#include "../../Utils/UsersUtils.h"
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
        for (size_t i =0; i < tokens.size(); ++i) {
            if (i>0){
                result += " ";
            }
            result += tokens[i];
        }
        return result;
    }

    CommandResult Rmgrp_Command(const vector<string>& tokens) {
        //Verificacion sesión activa y root
        if (!Global::sesionActual.activa) {
            return {false, "ERROR: No hay sesión activa"};
        }
        if (!Global::sesionActual.esRoot) {
            return {false, "ERROR: Solo root puede eliminar grupos"};
        }

        //Parseo -name
        string nombreGrupo;
        string atributos = joinTokens(tokens);
        static const regex lexic(R"(-name=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin=sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end =sregex_iterator();
        for (auto it= begin; it != end; ++it)
            found.push_back(it->str());

        if (found.size() != tokens.size()) {
            for (const auto& token : tokens) {
                if (!regex_search(token, lexic)) {
                    return {false, "ERROR: Parámetro no reconocido: " + token + " en RMGRP"};
                }
            }
        }

        for (const auto& fun : found){
            size_t eqPos = fun.find('=');
            if (eqPos == string::npos){
                return {false, "ERROR: formato inválido: " + fun};
            }

            string key =toLowerStr(fun.substr(0, eqPos));
            string value = fun.substr(eqPos + 1);
            if (value.size()>=2 && value.front() == '"' && value.back() == '"'){
                value= value.substr(1, value.size() - 2);
            }

            if (key== "-name") {
                if (value.empty()){
                    return {false, "ERROR: nombre de grupo vacío"};
                } 
                nombreGrupo = value;
            } 
            else{
                return {false, "ERROR: parámetro desconocido: " +key};
            }
        }
        if (nombreGrupo.empty()){
            return {false, "ERROR: falta -name"};
        }

        //Obtencion de partición, superbloque y leer users.txt
        string diskPath =Global::sesionActual.diskPath;
        string id =Global::sesionActual.idParticion;
        Estructuras::PARTITION mountedPart;
        string errMsg;
        if (!Global::GetMountedPartition(id, mountedPart, diskPath, errMsg)) {
            return {false, "ERROR: " +errMsg};
        }

        Estructuras::SUPERBLOCK sb;
        if (!Ext2Utils::LeerSuperbloque(diskPath, mountedPart.Partition_start, sb, errMsg)) {
            return {false, "ERROR: No se pudo leer superbloque: " + errMsg};
        }

        Estructuras::INODE inodeUsers;
        if (!Ext2Utils::LeerInodo(diskPath, sb, 1, inodeUsers, errMsg)) {
            return {false, "ERROR: No se pudo leer inodo de users.txt: " + errMsg};
        }
        string contenido;
        if (!Ext2Utils::LeerArchivo(diskPath, sb, inodeUsers, contenido, errMsg)) {
            return {false, "ERROR: No se pudo leer users.txt: " + errMsg};
        }

        vector<UsersUtils::Grupo> grupos;
        vector<UsersUtils::Usuario> usuarios;
        if (!UsersUtils::ParsearUsersTXT(contenido, grupos, usuarios, errMsg)) {
            return {false, "ERROR: Error al parsear users.txt: " + errMsg};
        }

        //Buscar el grupo y marcarlo como eliminado
        bool encontrado =false;
        for (auto& g : grupos) {
            if (g.nombre==nombreGrupo && !g.eliminado) {
                g.gid =0;
                g.eliminado= true;
                encontrado=true;
                break;
            }
        }

        if (!encontrado){
            return {false, "ERROR: Grupo '" + nombreGrupo + "' no existe o ya fue eliminado"};
        }

        //nuevo contenido
        string nuevoContenido= UsersUtils::GenerarUsersTXT(grupos, usuarios);
        inodeUsers.I_size=nuevoContenido.size();
        inodeUsers.I_mtime= static_cast<float>(time(nullptr));

        if (!Ext2Utils::EscribirArchivo(diskPath, sb, 1, inodeUsers, nuevoContenido, errMsg)) {
            return {false, "ERROR: No se pudo escribir users.txt: "+errMsg};
        }
        return {true, "RMGRP: Grupo '" + nombreGrupo + "' eliminado correctamente"};
    }
} 