#include "FDISK.h"
#include "../Str_Mbr/MBR.h"
#include "../Str_Ebr/EBR.h"
#include "../../Utils/Utilities.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <climits>
#include <cstring>

using namespace std;

namespace Estructuras{
    // Funciones auxiliares 
    namespace {

        string trimNulBoth(const string& s){
            size_t start= s.find_first_not_of('\0');
            if (start ==string::npos){
               return "";
            }
            size_t end= s.find_last_not_of('\0');
            return s.substr(start, end -start+1);
        }

        // Comparación case-insensitive
        bool equalFold(const string& a, const string& b) {
            if (a.size()!= b.size()) return false;
            for (size_t i=0; i <a.size(); ++i){
                if(tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])){
                    return false;
                }
            }
            return true;
        }

        // Verifica que el nombre no exista ya en las particiones delMBR
        bool nombreUnico(const MBR& mbr, const string& name) {
            for (int i=0; i< 4; ++i) {
                const PARTITION& p = mbr.Mbr_partitions[i];
                if (p.Partition_start !=-1) {
                    string pname(p.Partition_name, sizeof(p.Partition_name));
                    pname =trimNulBoth(pname);
                    if (equalFold(pname, name))
                        return false;
                }
            }
            return true;
        }

        //Conteo particiones primarias y extendidas
        void contarPE(const MBR& mbr, int& primarias, int& extendidas, bool& hayExtendida) {
            primarias =0;
            extendidas= 0;
            hayExtendida = false;
            for (int i=0; i < 4; ++i){
                const PARTITION& p = mbr.Mbr_partitions[i];
                if (p.Partition_start !=-1) {
                    if (p.Partition_type[0] == 'P'){
                        primarias++;
                    }

                    else if (p.Partition_type[0] =='E') {
                        extendidas++;
                        hayExtendida =true;
                    }
                }
            }
        }

        // Devuelve el offset donde se puede colocar la partición, o -1 si no hay espacio.
        int encontrarEspacio(const MBR& mbr, int size, char fit, char tipo){
            int discoSize =mbr.Mbr_size;
            int offset =(int)sizeof(MBR);
            vector<pair<int, int>> espacios; //<start, size>

            // Recordatorio de las particiones existentes (ordenadas por start)
            vector<PARTITION> parts;
            for (int i =0; i<4; ++i){
                if (mbr.Mbr_partitions[i].Partition_start != -1){
                    parts.push_back(mbr.Mbr_partitions[i]);
                }
            }
            sort(parts.begin(), parts.end(), [](const PARTITION& a, const PARTITION& b) {return a.Partition_start < b.Partition_start;});

            int current =offset;
            for (auto& p : parts) {
                if (p.Partition_start> current) {
                    espacios.push_back({current, p.Partition_start - current});
                }
                current =p.Partition_start + p.Partition_size;
            }
            if (current< discoSize) {
                espacios.push_back({current, discoSize - current});
            }

            // Si es lógica, solo usar espacio dentro de la extendida
            if (tipo =='L'){
                const PARTITION* ext= nullptr;
                for (auto& p : parts) {
                    if(p.Partition_type[0] =='E') {
                        ext =&p;
                        break;
                    }
                }
                if(!ext) {
                   return -1; 
                }

                //Filtro espacios que estén dentro de la extendida
                vector<pair<int, int>> espaciosLogicos;
                for (auto& e : espacios) {
                    int e_start = e.first;
                    int e_end = e_start +e.second;
                    int ext_start = ext->Partition_start;
                    int ext_end = ext_start + ext->Partition_size;
                    int int_start = max(e_start, ext_start);
                    int int_end= min(e_end, ext_end);
                    if (int_start < int_end){
                        espaciosLogicos.push_back({int_start, int_end - int_start});
                    }
                }
                espacios = espaciosLogicos;
            }

            //Aplicacion ajuste
            int mejorStart =-1;
            if(fit =='F'){ 
                for (auto& e : espacios) {
                    if (e.second >= size) {
                        mejorStart = e.first;
                        break;
                    }
                }
            } 
            else if (fit =='B') { //mejor Fit
                int mejorSize= INT_MAX;
                for (auto& e : espacios) {
                    if (e.second >= size && e.second < mejorSize) {
                        mejorSize = e.second;
                        mejorStart=e.first;
                    }
                }
            } 
            else if (fit =='W') { //peorr Fit
                int peorSize = -1;
                for (auto& e : espacios) {
                    if (e.second >= size && e.second >peorSize) {
                        peorSize= e.second;
                        mejorStart =e.first;
                    }
                }
            }
            return mejorStart;
        }

        // Escribe un EBR en una posición dada
        bool escribirEBR(const string& path, int pos, const EBR& ebr, string& errMsg) {
            fstream file(path, ios::binary | ios::in | ios::out);
            if(!file.is_open()) {
                errMsg ="No se pudo abir el disco para escribir EBR";
                return false;
            }
            file.seekp(pos, ios::beg);
            file.write(reinterpret_cast<const char*>(&ebr), sizeof(EBR));
            if (!file){
                errMsg= "Error al escribir EBR";
                return false;
            }
            return true;
        }

        //Lectura de unn EBR de una posición
        bool leerEBR(const string& path, int pos, EBR& ebr, string& errMsg){
            ifstream file(path, ios::binary);
            if (!file.is_open()){
                errMsg ="No se pudo abrir el disco paa leer EBR";
                return false;
            }
            file.seekg(pos, ios::beg);
            file.read(reinterpret_cast<char*>(&ebr), sizeof(EBR));
            if (!file){
                errMsg= "Error al leer EBR";
                return false;
            }
            return true;
        }
    }

//Función principal