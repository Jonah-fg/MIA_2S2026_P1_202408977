#include "HttpServer.h"
#include "../../httplib.h"
#include "../Analyzer/Analyzer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <streambuf>
using namespace std;

namespace Server {
    //Redireccion cout a un stringstream para capturar la salida de Analyzer
    static string ejecutarComando(const string& comando) {
        stringstream buffer;
        streambuf* oldCout = cout.rdbuf(buffer.rdbuf());
        Analyzer::Analyze(vector<string>{comando});
        cout.rdbuf(oldCout);
        return buffer.str();
    }

    static string ejecutarScript(const string& script) {
        stringstream buffer;
        streambuf* oldCout = cout.rdbuf(buffer.rdbuf());

        istringstream stream(script);
        string linea;
        while (getline(stream, linea)) {
            Analyzer::Analyze(vector<string>{linea});
        }
        cout.rdbuf(oldCout);
        return buffer.str();
    }
    static httplib::Server* svr = nullptr;

    void iniciarServidor(){
        svr =new httplib::Server();

        // Ruta POST /execute: recibe un comando o script y devuelve la salida
        svr->Post("/execute", [](const httplib::Request& req, httplib::Response& res){
            // Permitir CORS
            res.set_header("Access-Control-Allow-Origin","*");
            res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");

            string comando = req.body;
            string salida = ejecutarScript(comando);
            res.set_content(salida, "text/plain");
        });

        // Manejar preflight OPTIONS (para CORS)
        svr->Options("/execute", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.status = 204;
        });

        cout <<" Servidor HTTP escuchando en http://localhost:8080" << endl;
        cout << " Endpoint: POST /execute" << endl;
        svr->listen("localhost", 8080);
    }

    void detenerServidor() {
        if (svr){
            svr->stop();
            delete svr;
            svr = nullptr;
        }
    }
}