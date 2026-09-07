#include "SUPERBLOCK.h"
#include "../Str_Inode/INODE.h"
#include "../Str_Fileblock/FILEBLOCK.h"
#include "../Str_Folderblock/FOLDERBLOCK.h"
#include <cstring>
#include <ctime>
#include <algorithm>
using namespace std;

namespace Estructuras{

    bool SUPERBLOCK::Create_UsersTXT(const string& path, string& errMsg) {

        //Creacion inodo de la carpeta raíz (inodo 0)
        INODE rootInode;
        memset(&rootInode, 0,sizeof(rootInode));
        rootInode.I_uid =1;
        rootInode.I_gid= 1;
        rootInode.I_size=0;
        rootInode.I_atime= static_cast<float>(time(nullptr));
        rootInode.I_ctime= static_cast<float>(time(nullptr));
        rootInode.I_mtime = static_cast<float>(time(nullptr));
        rootInode.I_block[0]=Sb_blocks_count; //apunta al bloque 0
        for (int i= 1; i <15; ++i){
            rootInode.I_block[i] =-1;
        }
        rootInode.I_type[0] ='0';  
        rootInode.I_perm[0]= '7';
        rootInode.I_perm[1]= '7';
        rootInode.I_perm[2] ='7';

        if (!rootInode.Serialize(path, Sb_first_ino, errMsg)){
            return false;
        }

        if (!Update_Inode_Bitmap(path, errMsg)){
            return false;
        }

        Sb_inodes_count++;
        Sb_free_inodes_count--;
        Sb_first_ino += Sb_inode_size;

        //Creacion bloque de la carpeta raíz (FolderBlock)
        FOLDERBLOCK rootBlock;
        memset(&rootBlock, 0, sizeof(rootBlock));
        //Entrada 0:"."
        rootBlock.B_content[0].B_name[0] = '.';
        rootBlock.B_content[0].B_inodo = 0;
        // Entrada 1:".."
        rootBlock.B_content[1].B_name[0]= '.';
        rootBlock.B_content[1].B_name[1]= '.';
        rootBlock.B_content[1].B_inodo = 0;
        // Entrada 2: vacía (se llenará con "users.txt")
        rootBlock.B_content[2].B_name[0] = '-';
        rootBlock.B_content[2].B_inodo=-1;
        //Entrada 3: vacía
        rootBlock.B_content[3].B_name[0] ='-';
        rootBlock.B_content[3].B_inodo= -1;

        if (!Update_Block_Bitmap(path, errMsg)){
            return false;
        }
        if (!rootBlock.Serialize(path, Sb_first_blo, errMsg)){
            return false;
        }

        Sb_blocks_count++;
        Sb_free_blocks_count--;
        Sb_first_blo += Sb_block_size;

        //Preparacion contenido de users.txt
        string usersTXT= "1,G,root\n1,U,root,root,123\n";
  
        if (!rootInode.Deserialize(path, Sb_inode_start+0, errMsg)) {
            return false;
        }
        rootInode.I_atime = static_cast<float>(time(nullptr));
        if (!rootInode.Serialize(path, Sb_inode_start +0, errMsg)){
            return false;
        }

        //Releer folderblock raíz para añadir la entrada "users.txt"
        if (!rootBlock.Deserialize(path, Sb_block_start + 0, errMsg)){
            return false;
        }
        memset(rootBlock.B_content[2].B_name, 0, sizeof(rootBlock.B_content[2].B_name));
        strncpy(rootBlock.B_content[2].B_name, "users.txt", sizeof(rootBlock.B_content[2].B_name) - 1);
        rootBlock.B_content[2].B_inodo =Sb_inodes_count; //inodo 1

        if (!rootBlock.Serialize(path, Sb_block_start +0, errMsg)){
            return false;
        }
        //Creacion inodo de users.txt(inodo 1)
        INODE usersInode;
        memset(&usersInode, 0, sizeof(usersInode));
        usersInode.I_uid= 1;
        usersInode.I_gid =1;
        usersInode.I_size= static_cast<int32_t>(usersTXT.size());
        usersInode.I_atime= static_cast<float>(time(nullptr));
        usersInode.I_ctime=static_cast<float>(time(nullptr));
        usersInode.I_mtime= static_cast<float>(time(nullptr));
        usersInode.I_block[0]=Sb_blocks_count;
        for (int i = 1; i < 15; ++i){
            usersInode.I_block[i]=-1;
        }
        usersInode.I_type[0] ='1';//rchivo
        usersInode.I_perm[0]='7';
        usersInode.I_perm[1]='7';
        usersInode.I_perm[2] ='7';

        if (!Update_Inode_Bitmap(path, errMsg)) {
            return false;
        }
        if (!usersInode.Serialize(path, Sb_first_ino, errMsg)){
            return false;
        }
        Sb_inodes_count++;
        Sb_free_inodes_count--;
        Sb_first_ino += Sb_inode_size;

        //Creacion bloque de users.txt (FileBlock)
        FILEBLOCK usersBlock;
        memset(&usersBlock, 0, sizeof(usersBlock));
        size_t n=min(usersTXT.size(), sizeof(usersBlock.B_content));
        memcpy(usersBlock.B_content, usersTXT.data(), n);

        if (!usersBlock.Serialize(path, Sb_first_blo, errMsg)) {
            return false;
        }
        if (!Update_Block_Bitmap(path, errMsg)){
            return false;
        }
        Sb_blocks_count++;
        Sb_free_blocks_count--;
        Sb_first_blo += Sb_block_size;

        errMsg.clear();
        return true;
    }
} 