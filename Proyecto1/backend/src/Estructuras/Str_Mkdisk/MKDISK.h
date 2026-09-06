#pragma once
#include <string>
using namespace std;

namespace Estructuras{
    struct MKDISK {
        int Size =0;
        string Unit;
        string Fit;
        string Path;
    };
    bool Struct_MKDISK(const MKDISK& disk, string& errMsg);
    bool MakeDisk(const MKDISK& disk, long long sizeB, string& errMsg);
}