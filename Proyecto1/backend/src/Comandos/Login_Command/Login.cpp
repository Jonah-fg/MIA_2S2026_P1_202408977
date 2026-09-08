#include "Login.h"
#include "../../Global/MountedPartitions.h"
#include "../../Global/Sesion.h"
#include <regex>
#include "../../Utils/Ext2Utils.h"
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
        for (size_t i =0; i<tokens.size(); ++i) {
            if (i>0){
                result += " ";
            }
            result+= tokens[i];
        }
        return result;
    }

    CommandResult Login_Command(const vector<string>& tokens) {
    //Parsear parámetros: -user, -pass,-id
        string usuario;
        string pass;
        string id;
        string atributos=joinTokens(tokens);
        static const regex lexic(R"(-user=[^\s]+|-pass=[^\s]+|-id=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin =sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end=sregex_iterator();
        for (auto it= begin; it != end; ++it){
            found.push_back(it->str());

        }
        for (const auto& fun : found) {
            size_t eqPos =fun.find('=');
            if (eqPos== string::npos)
                return {false, "ERROR: formato inválido: " + fun};

            string key=toLowerStr(fun.substr(0, eqPos));
            string value =fun.substr(eqPos + 1);
            if (value.size() >= 2 && value.front()=='"' && value.back() == '"'){
                 value= value.substr(1, value.size() - 2 );
            }

            if(key == "-user") {
                if (value.empty()) {
                    return {false, "ERROR: usuario vacío"};
                }
                usuario =value;
            } 
            else if (key== "-pass") {
                if (value.empty()) {
                    return {false, "ERROR: contraseña vacía"};
                }
                pass =value;
            } 
            else if (key== "-id") {
                if (value.empty()) 
                return {false, "ERROR: id vacío"};

                id=value;
            } 
            else{
                return {false, "ERROR: parámetro desconocido: "+ key};
            }
        }

        if (usuario.empty()){
            return {false, "ERROR: falta -user"};
        } 
        if (pass.empty()){
            return{false, "ERROR: falta -pass"};
        } 
        if (id.empty()){
            return {false, "ERROR: falta -id"};
        }
        //Verificación que ya no haya sesión activa
        if (Global::sesionActual.activa){
            return{false, "ERROR: ya hay una sesión activa, ejecute logout primero"};
        }

        //Obtencion de la partición montada y el path del disco
        Estructuras::PARTITION mountedPart;
        string diskPath;
        string errMsg;
        if(!Global::GetMountedPartition(id, mountedPart, diskPath, errMsg)) {
            return {false, "ERROR: " +errMsg};
        }

        //Lectura del superbloque de la partición
        Estructuras::SUPERBLOCK sb;
        if (!Ext2Utils::LeerSuperbloque(diskPath, mountedPart.Partition_start, sb, errMsg)) {
            return {false, "ERROR: No se pudo leer superbloque: " +errMsg};
        }

        //Lectura del inodo 1 (users.txt)
        Estructuras::INODE inodeUsers;
        if(!Ext2Utils::LeerInodo(diskPath, sb, 1, inodeUsers, errMsg)) {
            return {false, "ERROR: No se pudo leer inodo de users.txt: " +errMsg};
        }

        //Lectura del contenido de users.txt
        string contenido;
        if(!Ext2Utils::LeerArchivo(diskPath, sb, inodeUsers, contenido, errMsg)) {
            return {false, "ERROR: No se pudo leer users.txt: " +errMsg};
        }

        //Parsear el archivo users.txt
        bool encontrado =false;
        int uidEncontrado= -1; 
        int gidEncontrado =-1;
        bool esRoot = false;

        istringstream iss(contenido);
        string linea;
        while (getline(iss, linea)) {
            linea.erase(remove(linea.begin(), linea.end(), '\r'), linea.end());
            if (linea.empty()){
                continue;
            }

            vector<string> campos;
            stringstream ss(linea);
            string campo;
            while (getline(ss, campo, ',')) {
                campo.erase(0, campo.find_first_not_of(" \t"));
                campo.erase(campo.find_last_not_of(" \t") + 1);
                campos.push_back(campo);
            }
            if (campos.size() <3) {
                continue;
            }

            //Si es grupo (G) se ignora
            if (campos[1] =="G"){
                continue;
            }
  
            if (campos.size() >= 5 && campos[1] =="U") {
                int uid =stoi(campos[0]);
                string grupo= campos[2];
                string user=campos[3];
                string password = campos[4];

                if (user ==usuario && password== pass) {
                    encontrado=true;
                    uidEncontrado =uid;
                    if (user== "root") {
                        esRoot=true;
                        gidEncontrado = 1;//GID de root
                    }
                    else{
                        // Se buscar el GID del grupo al que pertenece y se releemos el archivo 
                        gidEncontrado = 1;
                    }
                    break;
                }
            }
        }
        if(!encontrado) {
            return {false, "ERROR: usuario o contrasña incorrectos"};
        }

        //Activacion de sesión
        Global::sesionActual.activa =true;
        Global::sesionActual.usuario = usuario;
        Global::sesionActual.idParticion=id;
        Global::sesionActual.diskPath = diskPath;
        Global::sesionActual.uid =uidEncontrado;
        Global::sesionActual.gid= gidEncontrado;
        Global::sesionActual.esRoot=esRoot;
        return {true, "LOGIN: Sesión iniciada como '" +usuario + "'"};
    }

} 