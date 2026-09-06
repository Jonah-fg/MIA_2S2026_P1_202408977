#pragma once
#include <string>
#include <unordered_map>
#include "../Estructuras/Str_Mbr/MBR.h"
using namespace std;

namespace Global{
    extern unordered_map<string, string> MountedPartitions; // id=  path del disco

    bool GetEssentialRep(const string& id, Estructuras::MBR& mbrOut, string& diskPathOut, string& errMsg);
}