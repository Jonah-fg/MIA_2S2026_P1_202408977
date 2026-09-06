#include "FDISK.h"
#include "../Str_Mbr/MBR.h"
#include "../Str_Ebr/EBR.h"
#include "../../Utils/Utilities.h"
#include <algorithm>
#include <cstring>
#include <fstream>
using namespace std;

namespace Estructuras
{

    // Funciones auxiliares privadas de este archivo, una por cada tipo de particion que se puede crear con FDISK
    namespace
    {

        // Crea una particion PRIMARIA dentro del MBR del disco
        bool C_primaryPartition(const FDISK &fdisk, long long sizeBytes, string &errMsg)
        {
            //Se lee el MBR actual del disco para saber que particiones ya existen y cuanto espacio queda libre
            MBR part_mbr{};
            if (!part_mbr.DeserializeMBR(fdisk.Path, errMsg))
            {
                errMsg ="ERROR: No se pudo leer el MBR del disco: "+ errMsg;
                return false;
            }

            long long space_used= 0;
            int primaryParts =0;

            /* Se recorren las 4 particiones del MBR. Se ignoran las que
            estan libres (status== '2'). De las que si estan en uso,
            se suma su tamaño (space_used) y se cuentan las primarias
            y extendidas existentes, ya que ambas ocupan un slot de los 4 disponibles en el MBR*/
            for (int i=0; i<4; ++i)
            {
                const PARTITION &partition =part_mbr.Mbr_partitions[i];
                if (partition.Partition_status[0]!= '2')
                {
                    if (partition.Partition_type[0]== 'P' || partition.Partition_type[0]=='E')
                    {
                        space_used+= partition.Partition_size;
                        primaryParts++;
                    }
                }
            }

            //Espacio libre real= tamaño total del disco - lo ya usado
            long long free_space =static_cast<long long>(part_mbr.Mbr_size)- space_used;

            //Validaciones antes de crear la particion:
            if (primaryParts >=4)
            {
                errMsg = "ERROR: Ya existen 4 particiones en el disco (primarias o extedidas)";
                return false;
            }

            //La particion pedida no puede ser mas grande que el disco entero
            if (sizeBytes> part_mbr.Mbr_size)
            {
                errMsg = "ERROR: El tamaño de la particion es mayor al tamaño diponible en el disco";
                return false;
            }

            //La particion pedida no puede ser mas grande que el espacio libre
            if (sizeBytes > free_space)
            {
                errMsg ="ERROR: El tamaño de la particion es mayor al tamaño disponible en el disco";
                return false;
            }

            //Se busca el primer slot libre dentro del arreglo de 4 particiones.
            int startParticion =-1;
            int indexParticion = -1;
            string gerr;
            PARTITION *newParticion = part_mbr.GetFirstPartitionAvailable(startParticion, indexParticion, gerr);

            if (newParticion == nullptr)
            {
                errMsg = "ERROR: No se pudo obtener una particion disponible";
                return false;
            }

            // 6) Se llenan los datos de la particion en ese slot: status,
            //    tipo, ajuste, tamaño, nombre y byte de inicio en el disco
            newParticion->CreatePartition(startParticion, static_cast<int>(sizeBytes), fdisk.Type, fdisk.Fit, fdisk.Name);

            // 7) Se guarda el MBR actualizado de vuelta al disco
            if (!part_mbr.SerializeMBR(fdisk.Path, errMsg))
            {
                errMsg = "ERROR: No se pudo escribir el MBR en el disco: " + errMsg;
                return false;
            }
            return true;
        }

        // Crea la particion EXTENDIDA del disco
        bool C_extendedPartition(const FDISK &fdisk, long long sizeBytes, string &errMsg)
        {
            MBR part_mbr{};
            if (!part_mbr.DeserializeMBR(fdisk.Path, errMsg))
            {
                errMsg ="ERROR: No se pudo leer el MBR del disco: " + errMsg;
                return false;
            }

            //Se revisa si ya existe una particion extendida activa
            int extendedExists= 0;
            for (int i = 0; i <4; ++i)
            {
                const PARTITION &partition = part_mbr.Mbr_partitions[i];
                if (partition.Partition_status[0] != '2' && partition.Partition_type[0] == 'E')
                {
                    extendedExists++;
                }
            }

            if (extendedExists>0)
            {
                errMsg = "ERROR: Ya existe una particon extendida";
                return false;
            }

            if (sizeBytes >part_mbr.Mbr_size)
            {
                errMsg = "ERROR: El tamaño de la particion es mayor al tamaño disponible en el disco";
                return false;
            }

            //Igual que en la primaria: se busca un slot libre, se llena
            //con los datos de la particion y se guarda el MBR
            int startParticion = -1;
            int indexParticion =-1;
            string gerr;
            PARTITION *newParticion = part_mbr.GetFirstPartitionAvailable(startParticion, indexParticion,gerr);

            if (newParticion== nullptr)
            {
                errMsg="ERROR: No se pudo obtener una particion disponible";
                return false;
            }
            newParticion->CreatePartition(startParticion, static_cast<int>(sizeBytes),  fdisk.Type, fdisk.Fit, fdisk.Name);

            if (!part_mbr.SerializeMBR(fdisk.Path, errMsg))
            {
                errMsg= "ERROR: No se pudo escribir el MBR en el disco: " + errMsg;
                return false;
            }
            return true;
        }

        //Copia el nombre de la particion logica dentro del campo Partition_name del EBR, rellenando con ceros el resto
        static void fillEbrName(EBR &ebr, const string &name)
        {
            memset(ebr.Partition_name, 0, sizeof(ebr.Partition_name));
            size_t n= min(name.size(), sizeof(ebr.Partition_name));
            memcpy(ebr.Partition_name, name.data(), n);
        }


        // Crea una particion LOGICA dentro de la particion extendida.
        bool C_logicalPartition(const FDISK &fdisk, long long sizeBytes, string &errMsg)
        {
            MBR part_mbr{};
            if(!part_mbr.DeserializeMBR(fdisk.Path, errMsg))
            {
                errMsg= "ERROR: No se pudo leer el MBR del disco: "+errMsg;
                return false;
            }

            //Se busca la particion extendida dentro del MBR: la logica tiene que vivir dentro de ella, si no existe no hay donde crearla.
            PARTITION extendedPartition{};
            bool foundExtended = false;
            for (int i = 0; i<4; ++i)
            {
                if (part_mbr.Mbr_partitions[i].Partition_type[0]== 'E')
                {
                    extendedPartition = part_mbr.Mbr_partitions[i];
                    foundExtended =true;
                    break;
                }
            }

            if (!foundExtended)
            {
                errMsg = "ERROR: No existe una particion extendida";
                return false;
            }

            //La logica no puede ser mas grande que toda la extendida
            if (sizeBytes > extendedPartition.Partition_size)
            {
                errMsg ="ERROR: El tamaño de la particion logica es mayor al tamaño disponible en la particion extendida";
                return false;
            }

            //Se abre el archivo del disco en modo lectura/escritura binaria.
            fstream file(fdisk.Path, ios::binary | ios::in | ios::out);
            if (!file.is_open())
            {
                errMsg ="ERROR: No se pudo abrir el archivo del disco";
                return false;
            }

            //Se intenta leer el primer EBR, justo al inicio de la particion extendida.
            file.seekg(extendedPartition.Partition_start, ios::beg);

            EBR ebr{};
            file.read(reinterpret_cast<char *>(&ebr), sizeof(EBR));

            // caso1: todavia no hay ningun EBR en la extendida
            if (!file || ebr.Partition_size == 0)
            {
                file.clear();
                ebr =EBR{};
                ebr.Partition_mount[0] ='0';
                ebr.Partition_fit[0] =fdisk.Fit[0];
                ebr.Partition_start= extendedPartition.Partition_start;
                ebr.Partition_size =static_cast<int32_t>(sizeBytes);
                ebr.Partition_next=-1;
                fillEbrName(ebr, fdisk.Name);

                // Se escribe el EBR en el disco
                file.seekp(extendedPartition.Partition_start, ios::beg);
                file.write(reinterpret_cast<const char *>(&ebr), sizeof(EBR));
                if(!file)
                {
                    errMsg="ERROR: No se pudo escribir el EBR en la particion extedida";
                    return false;
                }

                // Justo despues del EBR (en el disco) va el contenido real de la particion logica
                int32_t logicalStart = extendedPartition.Partition_start +static_cast<int32_t>(sizeof(EBR));

                PARTITION logicalPartition{};
                logicalPartition.CreatePartition(static_cast<int>(logicalStart), static_cast<int>(sizeBytes), fdisk.Type, fdisk.Fit, fdisk.Name);
                // La logica hereda el mismo Partition_id que su extendida
                // contenedora (para identificar a que disco/particion
                // "padre" pertenece)
                memcpy(logicalPartition.Partition_id, extendedPartition.Partition_id, sizeof(logicalPartition.Partition_id));

                file.seekp(logicalStart, ios::beg);
                file.write(reinterpret_cast<const char*>(&logicalPartition), sizeof(PARTITION));
                if (!file)
                {
                    errMsg="ERROR: No se pudo escribir la particion logica";
                    return false;
                }
                file.close();

                // Se vuelve a guardar el MBR
                if (!part_mbr.SerializeMBR(fdisk.Path, errMsg)){
                    return false;
                }
                return true;
            } 

            // caso 2: ya existe al menos un EBR .
            long long size_used=ebr.Partition_size;

            while (ebr.Partition_next != -1)
            {
                file.seekg(ebr.Partition_next, ios::beg);
                file.read(reinterpret_cast<char *>(&ebr), sizeof(EBR));
                if (!file)
                {
                    errMsg = "ERROR: No se pudo leer el siguiente EBR";
                    return false;
                }
                size_used += ebr.Partition_size;
            }
            // Al salir del while, "ebr" contiene el ULTIMO EBR de la cadena

            // Se calcula cuanto espacio libre queda dentro de la extendida y se valida que la nueva logica quepa
            int32_t free_size = extendedPartition.Partition_size - static_cast<int32_t>(size_used);

            if (sizeBytes> free_size)
            {
                errMsg ="ERROR: El tamaño de la particion logica es mayor al tamaño disponible en la particion extendida";
                return false;
            }

            // El nuevo EBR se ubica justo despues del contenido de la ultima particion logica existente
            int32_t newEBRstart = ebr.Partition_start + ebr.Partition_size+ static_cast<int32_t>(sizeof(EBR));

            //Se actualiza el ULTIMO EBR existente para que apunte al nuevo 
            ebr.Partition_next= newEBRstart;
            file.clear();
            file.seekp(ebr.Partition_start, ios::beg);
            file.write(reinterpret_cast<const char *>(&ebr), sizeof(EBR));
            if (!file)
            {
                errMsg= "ERROR: No se pudo actualizar el EBR anterior con el nuevo";
                return false;
            }

            EBR ebrNew{};
            ebrNew.Partition_mount[0]= '0';
            ebrNew.Partition_fit[0] = fdisk.Fit[0];
            ebrNew.Partition_start=newEBRstart;
            ebrNew.Partition_size = static_cast<int32_t>(sizeBytes);
            ebrNew.Partition_next =-1;
            fillEbrName(ebrNew, fdisk.Name);

            file.seekp(ebrNew.Partition_start, ios::beg);
            file.write(reinterpret_cast<const char *>(&ebrNew), sizeof(EBR));
            if (!file)
            {
                errMsg= "ERROR: No se pudo escribir el nuevo EBR";
                return false;
            }

            //Justo despues del nuevo EBR va el contenido de la nueva particion logica
            int32_t logicalStart= newEBRstart+ static_cast<int32_t>(sizeof(EBR));

            PARTITION logicalPartition{};
            logicalPartition.CreatePartition(static_cast<int>(logicalStart), static_cast<int>(sizeBytes), fdisk.Type, fdisk.Fit, fdisk.Name);
            // Misma herencia de Partition_id que en el camino A.
            memcpy(logicalPartition.Partition_id, extendedPartition.Partition_id, sizeof(logicalPartition.Partition_id));

            file.seekp(logicalStart, ios::beg);
            file.write(reinterpret_cast<const char *>(&logicalPartition), sizeof(PARTITION));
            if (!file)
            {
                errMsg= "ERROR: No se pudo escribir la particion logica";
                return false;
            }
            file.close();
            return true;
        }

    }


    // Recibe los datos ya validados del comando FDISK (tamaño, unidad, path del disco, tipo, ajuste y nombre) y
    // despacha a la funcion correspondiente segun el tipo de particion
    // pedido
    bool Struct_FDISK(const FDISK &fdisk, string &errMsg)
    {
        long long sizeBytes= 0;
        if (!Utilities::ConvertBytes(fdisk.Size, fdisk.Unit, sizeBytes, errMsg))
        {
            errMsg = "ERROR: No se pudo convertir el size de la particion (" + errMsg + ")";
            return false;
        }

        if (fdisk.Type== "P")
        {
            return C_primaryPartition(fdisk, sizeBytes, errMsg);
        }
        else if (fdisk.Type == "E")
        {
            return C_extendedPartition(fdisk, sizeBytes, errMsg);
        }
        else if (fdisk.Type == "L")
        {
            return C_logicalPartition(fdisk, sizeBytes, errMsg);
        }

        errMsg ="ERROR: Tipo de particion no reconocido";
        return false;
    }
}
