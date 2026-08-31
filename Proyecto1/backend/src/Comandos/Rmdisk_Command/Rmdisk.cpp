#include "Rmdisk.h"
#include <regex>
#include <cstdio>
#include <algorithm>
#include <cctype>

using namespace std;

namespace Comandos {
    static string toLowerStr(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string>& tokens) {
        string result;
        for (size_t i= 0; i<tokens.size(); ++i) {
            if (i >0){
                result +=" ";
            } 
            result+= tokens[i];
        }
        return result;
    }

    CommandResult Rmdisk_Command(const vector<string>& tokens) {
        string atributos = joinTokens(tokens);
        static const regex lexic(R"(-path="[^"]+"|-path=[^\s]+)", regex::icase);

        vector<string> found;
        auto begin =sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end=sregex_iterator();
        for (auto it= begin; it!= end; ++it) {
            found.push_back(it->str());
        }

        if (found.size()!= tokens.size()) {
            for (const auto& token : tokens) {
                if (!regex_search(token, lexic)) {
                    return {false, "ERROR: Parámetro no reconocido: " + token + " en RMDISK"};
                }
            }
        }

        string pathVal;
        for(const auto& fun : found) {
            size_t eqPos=fun.find('=');
            if (eqPos ==string::npos){
                return {false, "ERROR: Parámetro inválido: " + fun};
            }
            string key =toLowerStr(fun.substr(0, eqPos));
            string value = fun.substr(eqPos + 1);
            if (value.size()>=2 && value.front() == '"' && value.back() == '"'){
                value = value.substr(1, value.size() - 2);
            }

            if(key == "-path") {
                if (value.empty()) return {false, "ERROR: Path vacío"};
                pathVal =value;
            } 
            else{
                return {false, "ERROR: Parámetro no reconocido: "+ key};
            }
        }

        if (pathVal.empty()){
            return {false, "ERROR: Falta -path"};
        } 

        if (remove(pathVal.c_str()) == 0){
            return {true, "RMDISK: Disco eliminado: " + pathVal};
        } 
        else{
            return {false, "ERROR: No se pudo elimiar el disco (puede que no exista)"};
        }
    }
}