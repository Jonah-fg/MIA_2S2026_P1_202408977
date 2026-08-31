#pragma once
#include <string>
using namespace std;

namespace Estructuras  {
    struct FDISK {
        int Size = 0;
        string Unit;
        string Path;
        string Type;
        string Fit;
        string Name;
    };
    bool Struct_FDISK(const FDISK& fdisk, string& errMsg);
}