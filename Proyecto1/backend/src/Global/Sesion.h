#ifndef SESION_H
#include <string>
#define SESION_H

namespace Global{
    struct Sesion{
        bool activa= false;
        std::string usuario;
        std::string idParticion; //ID de la partición montada
        std::string diskPath;
        int uid =-1;
        int gid= -1;
        bool esRoot= false;
    };
    extern Sesion sesionActual;
    void CerrarSesion();
}
#endif