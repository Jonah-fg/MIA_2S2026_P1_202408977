#include "Mkdisk.h"
#include <regex>
#include "../../Estructuras/Str_Mkdisk/MKDISK.h"
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

namespace Comandos
{
    static string toLowerStr(string s)
    {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

    static string joinTokens(const vector<string> &tokens)
    {
        string result;
        for (size_t i =0; i<tokens.size(); ++i)
        {
            if (i> 0){
                result+= " ";
            }
            result +=tokens[i];
        }
        return result;
    }

    CommandResult Mkdisk_Command(const vector<string> &tokens)
    {
        string atributos=joinTokens(tokens);
        static const regex lexic(R"(-size=\d+|-unit=[km]|-fit=[bfw]{2}|-path="[^"]+"|-path=[^\s]+)", regex::icase);

        // Busca todas las coincidencias del patron dentro de "atributos"
        vector<string> found;
        auto begin = sregex_iterator(atributos.begin(), atributos.end(), lexic);
        auto end = sregex_iterator();
        for (auto it=begin; it != end; ++it)
        {
            found.push_back(it->str());
        }

        //Si la cantidad de coincidencias no calza con la cantidad de tokens,
        //significa que algun token no es un parametro valido.
        if (found.size()!= tokens.size())
        {
            for (const auto &token : tokens)
            {
                if(!regex_search(token, lexic))
                {
                    return {false, "ERROR: Parametro no reconocido: " +token + " en comando MKDISK"};
                }
            }
        }
        bool hasSize = false;
        bool hasPath = false;
        int sizeVal = 0;
        string unitVal;
        string fitVal;
        string pathVal;

        //Se procesa cada parametro encontrado 
        for (const auto &fun : found)
        {
            size_t eqPos= fun.find('=');
            if (eqPos== string::npos)
            {
                return {false, "ERROR: Parametro invalido: " +fun};
            }

            //key=lo que esta antes del "=",                 value = lo que esta despues
            string key = toLowerStr(fun.substr(0, eqPos));
            string value = fun.substr(eqPos +1);

            //Si el valor viene entre comillas, se quitan 
            if (value.size() >= 2 && value.front() == '"'&& value.back() == '"')
            {
                value = value.substr(1, value.size() -2);
            }

            if (key =="-size")
            {
                try
                {
                    size_t charsUsados = 0;
                    int size = stoi(value, &charsUsados);
                    if (charsUsados != value.size() || size <= 0)
                    {
                        return {false, "ERROR: La capacidad del disco debe ser un numero entero positivo"};
                    }
                    sizeVal =size;
                    hasSize= true;
                }
                catch(...)
                {
                    return {false, "ERROR: La capacidad del disco debe ser un numero enter positivo"};
                }
            }
            else if (key == "-unit")
            {
                string v=value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v != "K" && v != "M")
                {
                    return {false, "ERROR: la unidad del disco debe ser K o M"};
                }
                unitVal = v;
            }
            else if (key == "-fit")
            {
                string v =value;
                transform(v.begin(), v.end(), v.begin(), ::toupper);
                if (v != "BF" && v!= "FF" && v != "WF")
                {
                    return {false,"ERROR: El ajuste del disco debe ser BF, FF o WF"};
                }
                fitVal = v;
            }
            else if (key== "-path")
            {
                if (value.empty())
                {
                    return{false, "ERROR: El path del disco no puede ser vacio"};
                }
                pathVal = value;
                hasPath = true;
            }
            else
            {
                return {false, "ERROR: Parametro no reconocido: "+ key};
            }
        }

        // Validaciones finales
        if (!hasSize)
        {
            return {false, "ERROR: La capacidad del disco no puede ser 0"};
        }
        if (!hasPath)
        {
            return {false, "ERROR: El path del disco no puede ser vacio"};
        }

        // Valores por defecto
        if (unitVal.empty())
        {
            unitVal = "M";
        }
        if (fitVal.empty())
        {
            fitVal = "FF";
        }

        //Aqui termina el analisis y empieza la creacion fisica
        Estructuras::MKDISK disk;
        disk.Size= sizeVal;
        disk.Unit=unitVal;
        disk.Fit = fitVal;
        disk.Path =pathVal;

        string creationErr;
        if (!Estructuras::Struct_MKDISK(disk, creationErr))
        {
            return {false, "ERROR: No se pudo crear el disco: " + creationErr};
        }
        ostringstream msg;
        msg << "MKDISK: Disco creado con exto -> " << "size=" << sizeVal << ", unit=" << unitVal << ", fit=" << fitVal<< ", path=" << pathVal;
        return {true, msg.str()};
    }
}