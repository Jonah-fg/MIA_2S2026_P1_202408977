#include "UsersUtils.h"
#include <sstream>
#include <algorithm>
using namespace std;
namespace UsersUtils {

    bool ParsearUsersTXT(const string& contenido, vector<Grupo>& grupos,vector<Usuario>& usuarios, string& errMsg){
        grupos.clear();
        usuarios.clear();

        istringstream iss(contenido);
        string linea;
        while (getline(iss, linea)){
            if (!linea.empty() && linea.back() =='\r'){
                linea.pop_back();
            }
            if (linea.empty()){
                continue;
            }
            vector<string> campos;
            stringstream ss(linea);
            string campo;
            while (getline(ss, campo, ',')) {
                //Quitar espacios
                campo.erase(0, campo.find_first_not_of(" \t"));
                campo.erase(campo.find_last_not_of(" \t")+1);
                campos.push_back(campo);
            }

            if (campos.size() <3) 
                continue;

            if (campos[1] =="G") {
                Grupo g;
                g.gid =stoi(campos[0]);
                g.nombre = campos[2];
                g.eliminado =(g.gid == 0);
                grupos.push_back(g);
            }
            else if (campos.size()>= 5 && campos[1] == "U") {
                Usuario u;
                u.uid = stoi(campos[0]);
                u.grupo= campos[2];
                u.nombre =campos[3];
                u.contrasena= campos[4];
                u.eliminado=(u.uid == 0);
                usuarios.push_back(u);
            }
        }
        errMsg.clear();
        return true;
    }

    string GenerarUsersTXT(const vector<Grupo>& grupos, const vector<Usuario>& usuarios) {
        stringstream ss;
        //Primero los grupos
        for (const auto& g : grupos){
            ss<< g.gid <<",G," << g.nombre << "\n";
        }
        //Luego los usuarios
        for (const auto& u : usuarios) {
            ss <<u.uid << ",U,"<< u.grupo << "," << u.nombre << ","<< u.contrasena << "\n";
        }
        return ss.str();
    }

    int SiguienteGID(const vector<Grupo>& grupos) {
        int maxGid =1; 
        for (const auto& g : grupos) {
            if (g.gid > maxGid && !g.eliminado) {
                maxGid=g.gid;
            }
        }
        return maxGid+1;
    }

    int SiguienteUID(const vector<Usuario>& usuarios) {
        int maxUid = 1;
        for (const auto& u : usuarios) {
            if (u.uid>maxUid && !u.eliminado) {
                maxUid = u.uid;
            }
        }
        return maxUid + 1;
    }
} 