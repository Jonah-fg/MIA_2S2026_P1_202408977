#ifndef USERS_UTILS_H
#define USERS_UTILS_H
#include <string>
#include <vector>

namespace UsersUtils{
    struct Grupo {
        int gid;
        std::string nombre;
        bool eliminado;  
    };

    struct Usuario {
        int uid;
        std::string grupo;     
        std::string nombre;
        std::string contrasena;
        bool eliminado;    // true si UID== 0
    };
    // Lee el archivo users.txt desde el disco y devuelve listas de grupos y usuarios
    bool ParsearUsersTXT(const std::string& contenido, std::vector<Grupo>& grupos, std::vector<Usuario>& usuarios, std::string& errMsg);

    // Genera el contenido de users.txt a partir de las litas
    std::string GenerarUsersTXT(const std::vector<Grupo>& grupos, const std::vector<Usuario>& usuarios);
    int SiguienteGID(const std::vector<Grupo>& grupos);
    int SiguienteUID(const std::vector<Usuario>& usuarios);
} 
#endif