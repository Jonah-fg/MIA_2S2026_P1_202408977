#include "Fdisk.h"
#include "../../Estructuras/Str_Fdisk/FDISK.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

namespace Comandos {
    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i =0; i <tokens.size(); ++i) {
            if (i> 0){
                result +=" ";  
            }
            
            result+= tokens[i];
        }
        return result;
    }

    CommandResult Fdisk_Command(const vector<string>& tokens) {
        string atributos = joinTokens(tokens);
        static const regex lexic(R"(-size=\d+|-unit=[bBkKmM]|-fit=[bBfF]{2}|-path="[^"]+"|-path=[^\s]+|-type=[pPeElL]|-name="[^"]+"|-name=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin =sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end =sregex_iterator();
        for (auto it =begin; it!= end; ++it)
            found.push_back(it->str());

        if (found.size()!= tokens.size()) {
            for (const auto& token : tokens) {
                if (!regex_search(token, lexic)) {
                    return {false, "ERROR: Parámetro no reconocido: " + token + " en FDISK"};
                }
            }
        }
        bool hasSize= false, hasPath = false, hasName = false;
        int sizeVal =0;
        string unitVal, fitVal, pathVal, typeVal, nameVal;

        for (const auto& fun : found) {
            size_t eqPos = fun.find('=');
            if (eqPos== string::npos) {
                return{false, "ERROR: Parámetro inválido: " + fun};
            }

            string key = toLowerStr(fun.substr(0, eqPos));
            string value = fun.substr(eqPos + 1);
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.size() - 2);

            if (key=="-size") {
                try{
                    size_t chars= 0;
                    int size = stoi(value, &chars);
                    if(chars != value.size() || size <= 0){
                        return {false, "ERROR: El size debe ser entero positivo"};
                    }
                    sizeVal =size;
                    hasSize =true;
                }
                catch (...){
                    return {false, "ERROR: El size debe ser entero positivo"};
                }
            } 
            else if(key == "-unit") {
                string v = value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v != "B" && v != "K" && v != "M")
                    return {false, "ERROR: Unit debe ser B, K o M"};
                unitVal = v;
            } 
            else if (key == "-path") {
                if (value.empty())
                    return {false, "ERROR: Path no puede estar vacío"};
                pathVal = value;
                hasPath = true;
            }
            else if (key == "-type") {
                string v = value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v != "P" && v != "E" && v != "L"){
                    return {false, "ERROR: Type debe ser P,E o L"};
                }
                typeVal =v;
            }
            else if (key == "-fit") {
                string v = value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v != "BF" && v != "FF" && v != "WF"){
                    return {false, "ERROR: Fit debe ser BF, FF o WF"};
                }
                fitVal = v;
            } 
            else if (key == "-name") {
                if (value.empty())
                    return {false, "ERROR: Name no puede estar vacío"};
                nameVal =value;
                hasName = true;
            } 
            else{
                return {false, "ERROR: Parámetro no reconocido: " + key};
            }
        }

        if (!hasSize) {
            return{false, "ERROR: Falta -size"};
        }
        if (!hasPath) {
            return {false, "ERROR: Falta -path"};
        }
        if (!hasName) {
            return {false, "ERROR: Falta -name"};
        }

        if (unitVal.empty()) {
            unitVal = "K";
        }

        if (typeVal.empty()) {
            typeVal = "P"; 
        }

        if (fitVal.empty()) 
            fitVal = "WF";

        Estructuras::FDISK fdisk;
        fdisk.Size = sizeVal;
        fdisk.Unit =unitVal;
        fdisk.Path = pathVal;
        fdisk.Type= typeVal;
        fdisk.Fit =fitVal;
        fdisk.Name=nameVal;

        string errMsg;
        if (!Estructuras::Struct_FDISK(fdisk, errMsg))
            return {false, "ERROR: "+ errMsg};

        return {true, "FDISK: Partición creada con xito"};
    }
}