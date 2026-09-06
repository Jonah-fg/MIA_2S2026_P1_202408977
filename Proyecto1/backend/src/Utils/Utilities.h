#pragma once
#include <string>
using namespace std;

namespace Utilities {
    bool ConvertBytes(int size, const string& unit, long long& outBytes, string& errMsg);
    extern const string Carnet;
    bool GetLetra(const string& path, string& letterOut, string& errMsg);
    void GetFileNames(const string& path, string& dotFileName, string& outputImage);
    bool CreateParentDir(const string& path, string& errMsg);

}