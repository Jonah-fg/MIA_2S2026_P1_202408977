#include "Logout.h"
#include "../../Global/Sesion.h"
using namespace std;
namespace Comandos{

    CommandResult Logout_Command(const vector<string>& /*tokens*/) {
        if (!Global::sesionActual.activa){
            return {false, "ERROR: No hay sesión activa"};
        }
        Global::CerrarSesion();
        return {true, "LOGOUT: Sesión cerrada con éxito"};
    }
}