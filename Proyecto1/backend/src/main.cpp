#include <iostream>
#include <string>
#include <vector>
#include "Analyzer/Analyzer.h"
#include "../httplib.h"
#include "Server/HttpServer.h"
using namespace std;

int main() {
    srand(time(nullptr));
    // Inicio servidor HTTP en un hilo separado
    thread serverThread([]() {
        Server::iniciarServidor();
    });
    serverThread.detach();
    this_thread::sleep_for(chrono::milliseconds(500));

    // Consola interactiva (opcional, pero útil para pruebas)
    cout << "----------------------- Simulador de Discos MIA ------------------" << endl;
    cout << "Comandos disponibles: mkdisk, rmdisk, fdisk, mount, mkfs, login, logout," << endl;
    cout << "  mkgrp, rmgrp, mkusr, rmusr, chgrp, mkfile, mkdir, cat, rep" << endl;
    cout << "Escribe 'salir' para terminar."<< endl;
    cout << "El servidor HTTP está activo en http://localhost:8080"<< endl;

    string linea;
    while (true) {
        cout << "\n> ";
        getline(cin, linea);
        if (linea == "salir") 
        {
            break;
        }
        if (linea.empty()) 
        {
            continue;
        }
        Analyzer::Analyze(vector<string>{linea});
    }
    cout << "Saliendo, adiooooiooooooooooooooooos" << endl;
    Server::detenerServidor();
    return 0;
}