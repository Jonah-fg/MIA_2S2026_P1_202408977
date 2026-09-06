#include "Utilities.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <unordered_map>
#include <vector>
using namespace std;

namespace Utilities {
    const std::string Carnet="77";

    namespace {
        unordered_map<string, string> pathToLetter;
        const vector<string> Alfabeto={"A","B","C","D", "E", "F", "G", "H","I", "J","K", "L", "M","N","O","P","Q", "R", "S", "T","U", "V","W", "X", "Y","Z"};
        int nextLetterIndex= 0;
    }

    bool GetLetra(const string& path, string& letterOut, string& errMsg) {
        auto it= pathToLetter.find(path);
        if (it== pathToLetter.end()){
            if (nextLetterIndex <(int)Alfabeto.size()){
                pathToLetter[path] =Alfabeto[nextLetterIndex];
                nextLetterIndex++;
            } 
            else{
                errMsg= "No hay más letrs disponibles para nuevos discos.";
                return false;
            }
        }
        letterOut =pathToLetter[path];
        errMsg.clear();
        return true;
    }

    bool ConvertBytes(int size, const string& unit, long long& outBytes, string& errMsg) {
    string u = unit;
    transform(u.begin(), u.end(), u.begin(), [](unsigned char c) { return toupper(c); });
    if (u == "B"){
        outBytes =(long long)size;
    } 
    else if (u == "K") {
        outBytes= (long long)size * 1024LL;
    } 
    else if (u=="M" || u.empty()) {
        outBytes = (long long)size * 1024LL * 1024LL;
    }
    else{
        errMsg = "Unidad inválida: " + unit;  
        return false;
    }
    return true;
}

    bool CreateParentDir(const std::string& path, std::string& errMsg) {
        namespace fs= std::filesystem;
        fs::path dir= fs::path(path).parent_path();
        if (!dir.empty()){
            std::error_code ec;
            fs::create_directories(dir, ec);
            if (ec){
                errMsg= "No se pudo crear el directorio padre: " +dir.string();
                return false;
            }
        }
        return true;
    }

    void GetFileNames(const std::string& path, std::string& dotFileName, std::string& outputImage) {
        namespace fs=std::filesystem;
        outputImage =path;
        fs::path p(path);
        p.replace_extension(".dot");
        dotFileName= p.string();
    }
}