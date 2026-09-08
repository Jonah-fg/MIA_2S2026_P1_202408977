#include "Analyzer.h"
#include "../Comandos/CommandResult.h"
#include "../Comandos/Mkdisk_Command/Mkdisk.h"
#include "../Comandos/Fdisk_Command/Fdisk.h"
#include "../Comandos/Mount_Command/Mount.h"
#include <iostream>
#include "../Global/MountedPartitions.h"
#include "../Comandos/Rmdisk_Command/Rmdisk.h"
#include "../Comandos/Mkfs_Command/Mkfs.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include "../Comandos/Login_Command/Login.h"
#include "../Comandos/Logout_Command/Logout.h"

using namespace std;

namespace Analyzer {
    static string trim(const string& s) {
        size_t start= s.find_first_not_of(" \t\r\n");
        if (start ==string::npos){
            return "";
        }
        size_t end= s.find_last_not_of(" \t\r\n");
        return s.substr(start, end-start + 1);
    }

    static vector<string> fields(const string& s){
        vector<string> tokens;
        istringstream iss(s);
        string tok;
        while (iss>> tok){
            tokens.push_back(tok);
        }
        return tokens;
    }

    static string toLower(string s){
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return tolower(c); }); //funcion minusculas 
        return s;
    }

    void Analyze(const vector<string>& inputs){
        if (inputs.empty()) {
            cout <<"[ERROR] No se proporcionó ningn comando" << endl;
            return;
        }

        string input =trim(inputs[0]);
        if (input.empty()) {
            cout <<"\n" <<input << "\n" << endl;
            return;
        }

        vector<string> tokens= fields(input);
        if (tokens.empty()) {
            cout <<"[ERROR] No se proporcionó ningún comando válido" << endl;
            return;
        }

        tokens[0]=toLower(tokens[0]);
        vector<string> params(tokens.begin()+1, tokens.end());

        bool hasError =false;
        string errorMsg;
        string msg;

        if (tokens[0] =="mkdisk"){
            Comandos::CommandResult result = Comandos::Mkdisk_Command(params);
            if (result.success) {
                msg =result.message;
            }
            else{ 
                hasError= true; 
                errorMsg =result.message; 
            }
        } 
        else if(tokens[0] =="rmdisk"){
            Comandos::CommandResult result =Comandos::Rmdisk_Command(params);
            if (result.success) {
                msg= result.message;
            }
            else{ 
                hasError = true;
                errorMsg =result.message; 
            }
        } 
        else if (tokens[0]== "fdisk"){
            Comandos::CommandResult result= Comandos::Fdisk_Command(params);
            if (result.success) {
                msg =result.message;
            }
            else { 
                hasError= true; 
                errorMsg =result.message;
             }
        }
        else if (tokens[0]== "mount") {
            Comandos::CommandResult result=Comandos::Mount_Command(params);
            if (result.success) {
                msg =result.message;
            }
            else{ 
                hasError= true; 
                errorMsg =result.message; 
            }
        } 
        else if (tokens[0]=="mounted"){
            // Comando mounted: listar particiones montadas
            if (Global::MountedPartitions.empty()) {
                msg ="No hay particiones montadas.";
            }
            else {
                msg ="Particiones montadas:\n";
                for(auto& kv : Global::MountedPartitions) {
                    msg+=" ID: " + kv.first+ " -> Disco: " + kv.second + "\n";
                }
            }
        } 
        else if (tokens[0]== "mkfs") {
            Comandos::CommandResult result= Comandos::Mkfs_Command(params);
            if (result.success) {
                msg =result.message;
            }
            else{
                hasError= true;
                errorMsg =result.message;
            }
        }
        else if (tokens[0]== "login") {
            Comandos::CommandResult result= Comandos::Login_Command(params);
            if (result.success) {
                msg =result.message;
            }
            else{
                hasError= true;
                errorMsg =result.message;
            }
        }
        else if (tokens[0]== "logout") {
            Comandos::CommandResult result= Comandos::Logout_Command(params);
            if (result.success) {
                msg =result.message;
            }
            else{
                hasError = true;
                errorMsg =result.message;
            }
        }
        
        
        else{
            hasError= true;
            errorMsg = "Comando no reconocido: " + tokens[0];
        }
        if (hasError){
            cout <<"[ERROR] "<< errorMsg <<endl;
        }
        else
            cout <<msg <<endl;
    }
}