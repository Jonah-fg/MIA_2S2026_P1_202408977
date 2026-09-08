#include "Sesion.h"

namespace Global{
    Sesion sesionActual;

    void CerrarSesion() {
        sesionActual.activa=false;
        sesionActual.usuario.clear();
        sesionActual.idParticion.clear();
        sesionActual.diskPath.clear();
        sesionActual.uid= -1;
        sesionActual.gid = -1;
        sesionActual.esRoot=false;
    }
}