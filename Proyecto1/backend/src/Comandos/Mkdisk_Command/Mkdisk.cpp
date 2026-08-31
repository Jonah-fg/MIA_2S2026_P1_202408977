#include "Mkdisk.h"
#include "../../Estructuras/Str_Mkdisk/MKDISK.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

namespace Comandos {
    static string toLowerStr(string s){
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i = 0; i<tokens.size(); ++i) {
            if (i >0) result +=" ";
            result +=tokens[i];
        }
        return result;
    }

    CommandResult Mkdisk_Command(const vector<string>& tokens) {
        string atributos = joinTokens(tokens);
        static const regex lexic(R"(-size=\d+|-unit=[km]|-fit=[bfw]{2}|-path="[^"]+"|-path=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin =sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end=sregex_iterator();
        for(auto it = begin; it!= end; ++it){
            found.push_back(it->str());
        }

        if (found.size()!= tokens.size()) {
            for (const auto& token : tokens) {
                if (!regex_search(token, lexic)) {
                    return {false,"ERROR: Parámetro no reconocido: " + token + " en MKDISK"};
                }
            }
        }

        bool hasSize =false;
        bool hasPath = false;
        int sizeVal=0;
        string unitVal;  
        String fitVal; 
        String pathVal;
        for (const auto& fun :found) {
            size_t eqPos =fun.find('=');
            if (eqPos==string::npos){
                return {false, "ERROR: Parámetro inválido: " + fun};
            }

            string key =toLowerStr(fun.substr(0, eqPos));
            string value =fun.substr(eqPos + 1);
            if (value.size() >= 2 && value.front() =='"'&& value.back() == '"') {
                value = value.substr(1, value.size()- 2);
            }

            if (key =="-size") {
                try{
                    size_t chars= 0;
                    int size =stoi(value, &chars);

                    if (chars!= value.size() || size <= 0) {
                        return{false, "ERROR: El size debe ser un entero positivo"};
                    }
                    sizeVal =size;
                    hasSize=true;
                } 
                catch (...){
                    return {false,"ERROR: El size debe ser un entero positivo"};
                }
            } 
            else if (key== "-unit") {
                string v= value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v !="K" &&v!= "M") {
                    return {false, "ERROR: Unit debe ser K o M"};
                }
                unitVal =v;
            } 
            else if (key== "-fit") {
                string v= value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v!= "BF" && v != "FF" && v != "WF"){
                    return {false, "ERROR: Fit debe se BF, FF o WF"};
                }
                fitVal=v;
            }
            else if (key== "-path") {
                if (value.empty()) {
                    return {false, "ERROR: Path no puede estar vacío"};
                }
                pathVal = value;
                hasPath = true;
            }
            else{
                return {false, "ERROR: Parámetro no reconocido: "+ key};
            }
        }

        if (!hasSize) {
            return {false, "ERROR: Fala -size"};
        } 
    
        if (!hasPath){
            return {false, "ERROR: Falta -path"};
        }

        if (unitVal.empty()) {
            unitVal= "M";
        }

        if (fitVal.empty()){
            fitVal ="FF";
        }

        // Ejecucion de creacion
        Estructuras::MKDISK disk;
        disk.Size=sizeVal;
        disk.Unit= unitVal;
        disk.Fit= fitVal;
        disk.Path =pathVal;

        string errMsg;
        if (!Estructuras::Struct_MKDISK(disk, errMsg)) {
            return {false, "ERROR: "+ errMsg};
        }

        ostringstream msg;
        msg <<"MKDISK: Disco creado con éxito -> size=" <<sizeVal << ", unit=" << unitVal << ", fit="<< fitVal << ", path="<< pathVal;
        return {true, msg.str()};
    }
}