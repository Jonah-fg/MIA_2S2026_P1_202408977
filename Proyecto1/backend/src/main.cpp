#include <iostream>
#include <string>
#include <vector>
#include "Analyzer/Analyzer.h"
using namespace std;

int main() {
    cout << "-----------------Simulador de Discos MIA --------------" << endl;
    cout << "Comandos disponibles: mkdisk, rmdisk, fdisk, mount, mounted" << endl;
    cout << "Escribe 'salir' para terminar." << endl;

    string linea;
    while (true){
        cout << "\n> ";
        getline(cin, linea);
        if (linea == "salir"){
           break;
        }
        // Pasa la línea como un vector con un solo elemento
        Analyzer::Analyze(vector<string>{linea});
    }

    cout << "Saliendo..." << endl;
    return 0;
}