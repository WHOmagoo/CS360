//
// Created by whomagoo on 4/13/18.
//

#include <stdio.h>
#include <memory.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>

#define BLKSIZE 1024

typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

//struct ext2_super_block {
//    u32  s_inodes_count;       // total number of inodes
//    u32  s_blocks_count;       // total number of blocks
//    u32  s_r_blocks_count;
//    u32  s_free_blocks_count;  // current number of free blocks
//    u32  s_free_inodes_count;  // current number of free inodes
//    u32  s_first_data_block;   // first data block in this group
//    u32  s_log_block_size;     // 0 for 1KB block size
//    u32  s_log_frag_size;
//    u32  s_blocks_per_group;   // 8192 blocks per group
//    u32  s_frags_per_group;
//    u32  s_inodes_per_group;
//    u32  s_mtime;
//    u32  s_wtime;
//    u16  s_mnt_count;          // number of times mounted
//    u16  s_max_mnt_count;      // mount limit
//    u16  s_magic;              // 0xEF53

int filep = 0;

GD    *gp;
SUPER *sp;

//cwd is the current iNode or cwd and should point at iBuf
INODE *cwd;

char wd[BLKSIZE];

char gBuf[BLKSIZE];
char sBuf[BLKSIZE];
char iBuf[BLKSIZE];
char dBuf[BLKSIZE];

//a should be the non null terminated string, b should be null terminated
int my_strcmp(char *a, char *b, __u8 len){
    if(a == 0){
        if(b == 0){
            return 0;
        } else {
            return 1;
        }
    } else if(b == 0){
        return 1;
    }


    while(len > 0){
        if(*a == '\0' && *b == '\0'){
            return 0;
        }

        if(*a != *b){
            return 1;
        }

        a++;
        b++;
        len--;
    }

    if(*b == '\0'){
        return 0;
    } else {
        return 1;
    }

}

void printName(char *name, int len){
    if(name == 0){
        return;
    }

    while(len > 0 && *name != '\0'){
        printf("%c", *name);
        name++;
        len--;
    }
}

int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd,(long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int set_block(int fd, int blk, char buf[])
{
    lseek(fd, (long)blk*BLKSIZE, 0);
    write(fd, buf, BLKSIZE);
}

int init(int fd){
    get_block(fd, 1, sBuf);
    sp = (SUPER *) sBuf;

    get_block(fd, 2, gBuf);
    gp = (GD *) gBuf;

    get_block(fd, gp->bg_inode_table, iBuf);

    cwd = (INODE *) iBuf + 1;
}

void getTime(int time, char *buf){
    time_t timeStamp = time;
    struct tm *ts = localtime(&timeStamp);
    strftime(buf, 80, "%Y-%m-%d %H:%M", ts);
}

void bufcpy(char bufTo[BLKSIZE], const char bufFrom[BLKSIZE]){
    for(int i = 0; i < BLKSIZE; i++){
        bufTo[i] = bufFrom[i];
    }
}

INODE * searchInDirectBlock(char *name, char buf[]){
    //TODO refactor to here
}

//looks in buf to find the specified file
INODE * getInBlock(char *file, char buf[BLKSIZE], INODE *curDir, int type) {
    int found = 0;

    int curBlock[15];

    for (int i = 0; i < 15; i++) {
        curBlock[i] = curDir->i_block[i];
    }

    for (int i = 0; i < 12 && !found; i++) {
        char nextDir[BLKSIZE];

        if (curBlock[i] == 0) {
            break;
        }

        get_block(filep, curBlock[i], nextDir);

        int len = 0;

        while (len < BLKSIZE) {
            DIR *dp = (DIR *) (nextDir + len);
            char *name = dp->name;

            if (my_strcmp(dp->name, file, dp->name_len) == 0) {

                char buftmp[BLKSIZE];
                get_block(filep, gp->bg_inode_table + (dp->inode - 1) / 8, buftmp);
                INODE *dirTmp = (INODE *) buftmp + (dp->inode - 1) % 8;

                if(dp->file_type == type || (dirTmp->i_mode >> 14) % 2 == 1) {
                    bufcpy(buf, buftmp);
                    curDir = (INODE *) ((u_int32_t) buf + (u_int32_t) dirTmp - (u_int32_t) buftmp);
                    return curDir;
                } else {
                    printf("Could not obtain %s, it was the wrong type.\nLooking for a %d type, it was a %d type", file, type, dp->file_type);
                    return 0;
                }

//                if (curDir->i_mode >> 14 % 2 == 1|| dp->file_type == 2) {
//                    //break out of the for loop and the while loop
//                    found = 1;
//                    break;
//                } else {
//                    //break out of the for loop and the while loop
//                    //found the next node but it wasn't a directory
//                    printf("%s was not found in the directory. ", file);
//                    i = 69;
//                    break;
//                }


            }
            len += dp->rec_len;
        }
    }

    //TODO singly and doubly linked items

    if(!found){
        printf("Couldn't find %s\n", file);
        return 0;
    }
}

INODE * getDir(char *dir, char buf[BLKSIZE]){
    char curBuf[BLKSIZE];
    INODE *curDir;

    if(dir == 0){
        bufcpy(buf, iBuf);
        return (INODE *) ((u_int32_t )buf + (u_int32_t )cwd - (u_int32_t )iBuf);
    }

    if(*dir == '/'){
        dir++;
        //copy root into curBuf
        get_block(filep, gp->bg_inode_table, curBuf);
        curDir = (INODE *) curBuf + 1;
    } else {
        //copy cwd into curBuf
        bufcpy(curBuf, iBuf);
        curDir = (INODE *) ((u_int32_t ) curBuf + (u_int32_t ) cwd - (u_int32_t ) iBuf);
    }


    char *lookingFor = strtok(dir, "/");
    int finished = 0;

    while (lookingFor != 0) {
        INODE * nextDir = getInBlock(lookingFor, curBuf, curDir, 2);
        if(nextDir && (nextDir->i_mode >> 14) % 2 == 1) {
                curDir = nextDir;
        } else {
            printf("Unable to find %s, %u\n", lookingFor, nextDir);
            return 0;
        }
//        int found = 0;
//
//        int curBlock[15];
//
//        for (int i = 0; i < 15; i++) {
//            curBlock[i] = curDir->i_block[i];
//        }
//
//        for (int i = 0; i < 12 && !found; i++) {
//            char nextDir[BLKSIZE];
//
//            if (curBlock[i] == 0) {
//                break;
//            }
//
//            get_block(filep, curBlock[i], nextDir);
//
//            int len = 0;
//
//            while (len < BLKSIZE) {
//                DIR *dp = (DIR *) (nextDir + len);
//                char *name = dp->name;
//
//                if (my_strcmp(dp->name, lookingFor, dp->name_len) == 0) {
//
//                    get_block(filep, gp->bg_inode_table + (dp->inode - 1) / 8, curBuf);
//                    curDir = (INODE *) curBuf + (dp->inode - 1) % 8;
//
//                    if (curDir->i_mode >> 14 % 2 || dp->file_type == 2) {
//                        //break out of the for loop and the while loop
//                        found = 1;
//                        break;
//                    } else {
//                        //break out of the for loop and the while loop
//                        //found the next node but it wasn't a directory
//                        printf("%s was not a directory. ", lookingFor);
//                        i = 69;
//                        break;
//                    }
//
//
//                }
//                len += dp->rec_len;
//            }
//        }

        //TODO implement singly and doubly indirect blocks
        
        
        lookingFor = strtok(NULL, "/");
        
    }


    bufcpy(buf, curBuf);
    return (INODE *) ((u_int32_t )buf + (u_int32_t )curDir - (u_int32_t ) curBuf);


}


//returns a pointer to the part after the last '/', if there are none, it returns 0;
char *splitLast(char *string){
    char copy[BLKSIZE];
    char *ptr = copy, *last = copy;
    strcpy(copy, string);

    ptr = strtok(copy, "/");

    while(ptr != 0){
        last = ptr;
        ptr = strtok(NULL, "/");
    }



    u_int32_t offset = (u_int32_t ) last - (u_int32_t) copy;
    if(offset == 0 || offset == 1) {
        return 0;
    } else {

        *((char *) ((u_int32_t) string + offset) - 1) = '\0';
        return (char *) ((u_int32_t) string + offset);
    }
}

void printSingleLinkedBlock(int blockNumber){
    char buf[BLKSIZE];
    get_block(filep, blockNumber, buf);
    int *blocksPtr = (int *) buf;
    int indirectBlocks[256];

    int end = 0;
    for (int i = 0; i < 256 && !end; ++i) {
        end = *blocksPtr == 0;
        indirectBlocks[i] = *blocksPtr;
        blocksPtr++;
    }

    for (int i = 0; i < 256 && indirectBlocks[i] != 0; i++) {
        get_block(filep, indirectBlocks[i], buf);
        printf("%s", buf);
    }
}

void printDoublyLinkedBlock(int blockNumber){
    char buf[BLKSIZE];
    get_block(filep, blockNumber, buf);
    int *blocksPtr = (int *) buf;
    int indirectBlocks[256];

    int end = 0;
    for (int i = 0; i < 256 && !end; ++i) {
        end = *blocksPtr == 0;
        indirectBlocks[i] = *blocksPtr;
        blocksPtr++;
    }

    for (int i = 0; i < 256 && indirectBlocks[i] != 0; i++) {
        printSingleLinkedBlock(indirectBlocks[i]);
    }
}

void printTriplyLinkedBlock(int blockNumber){
    char buf[BLKSIZE];
    get_block(filep, blockNumber, buf);
    int *blocksPtr = (int *) buf;
    int indirectBlocks[256];

    int end = 0;
    for (int i = 0; i < 256 && !end; ++i) {
        end = *blocksPtr == 0;
        indirectBlocks[i] = *blocksPtr;
        blocksPtr++;
    }

    for (int i = 0; i < 256 && indirectBlocks[i] != 0; i++) {
        printDoublyLinkedBlock(indirectBlocks[i]);
    }
}

int getParent(INODE **inode, char buf[BLKSIZE]){
    get_block(filep, (*inode)->i_block[0], buf);



    DIR *parentRef = (DIR *) buf;
    int curInode = parentRef->inode;
    parentRef = (DIR *) (buf + parentRef->rec_len);

    int parentInode = parentRef->inode;

    get_block(filep, gp->bg_inode_table + (parentInode - 1) / 8, buf);
    *inode = (INODE *) buf + (parentInode - 1) % 8;

    return curInode;
}

char *getName(INODE *inode, char buf[BLKSIZE]){
    int iNodeNum = getParent(&inode, buf);
    //inode = (INODE *) buf;

    int blocks[15];
    for(int i = 0; i < 15; i++){
        blocks[i] = inode->i_block[i];
        if(blocks[i] == 0){
            break;
        }
    }

    for(int i = 0; i < 13; i++){
        if(blocks[i] == 0){
            break;
        }

        get_block(filep, blocks[i], buf);
        int len = 0;
        while (len < BLKSIZE) {
            DIR *dp = (DIR *) (buf + len);
            len += dp->rec_len;

            if(dp->inode == iNodeNum){
                *(dp->name + dp->name_len) = (char) '/0';
                return dp->name;
            }
        }
    }

    //TODO implement singly and doubly indirect blocks

}

void pwd(){
    char parentBuf[BLKSIZE];

    bufcpy(parentBuf, iBuf);

    INODE *cur = (INODE *) ((u_int32_t ) parentBuf + (u_int32_t) cwd - (u_int32_t) iBuf);

    printf("%s", getName(cur, parentBuf));

}

void cat(char *location){
    if(location == 0){
        printf("No location specified\n");
        return;
    }

    //trim excess parameters
    strtok(location, " ");


    char *fileName = splitLast(location);

    if(fileName == 0) {
        if(*location == '/'){
            fileName = location + 1;
            location = "/";
        } else {
            fileName = location;
            location = 0;
        }
    }


    char buf[BLKSIZE];

    INODE *dir = getDir(location, buf);

    if(dir) {

        INODE *file = getInBlock(fileName, buf, dir, 1);

        if(file) {

            int blocks[15];

            for (int i = 0; i < 15; i++) {
                blocks[i] = file->i_block[i];
            }

            for (int i = 0; i < 15; i++) {
                if (blocks[i] == 0) {
                    break;
                }

                get_block(filep, blocks[i], buf);

                printf("%s", buf);
            }

            //singly indirect blocks
            if (blocks[12] != 0) {
                printSingleLinkedBlock(blocks[12]);
            }

            if (blocks[13] != 0) {
                printDoublyLinkedBlock(blocks[13]);
            }

            if (blocks[14] != 0) {
                printTriplyLinkedBlock(blocks[14]);
            }

            printf("\n");
        }
    }
}

void cd(char *dir){
    if(dir == 0){
        printf("No location specified\n");
        return;
    }

    //remove excess parameters

    strtok(dir, " ");

    char buf[BLKSIZE];

    INODE *newDir = getDir(dir, buf);

    if(newDir != 0){
        bufcpy(iBuf, buf);
        cwd = (INODE *) ((u_int32_t )iBuf + (u_int32_t )newDir - (u_int32_t ) buf);
//may be implemented to keep track of cwd
//        if(*dir == '/'){
//            strcpy(wd, dir);
//        } else {
//
//        }
    }

}

void ls(char *dir) {

    char *curBuf[BLKSIZE];
    INODE *curInode;

    curInode = getDir(dir, curBuf);
    if(curInode) {

        int curBlock[15];

        for (int i = 0; i < 15; i++){
            curBlock[i] = curInode->i_block[i];
        }

        for (int i = 0; i < 12; i++) {
            char buf[BLKSIZE];

            if (curBlock[i] == 0) {
                break;
            }

            get_block(filep, curBlock[i], buf);

            int len = 0;

            while (len < BLKSIZE) {
                DIR *dp = (DIR *) (buf + len);
                len += dp->rec_len;

                char name[BLKSIZE];

                strcpy(name, dp->name);

                char curItemBuf[BLKSIZE];
                get_block(filep, gp->bg_inode_table + (dp->inode - 1) / 8, curItemBuf);

                curInode = (INODE *) curItemBuf + (dp->inode - 1) % 8;

                char timeBuf[80];
                getTime(curInode->i_ctime, timeBuf);


                char permissions[11] = "-rwxrwxrwx";
                int mode = curInode->i_mode;

                for (int i = 9; i > 0; i--) {
                    if (mode % 2 == 0) {
                        permissions[i] = '-';
                    }

                    mode = mode >> 1;
                }

                if((mode >> 5) % 2 == 1){
                    permissions[0] = 'd';
                }

                switch (dp->file_type) {
                    case 2:
                        permissions[0] = 'd';
                        break;
                    case 7:
                        permissions[0] = 'l';
                }

                printf("\t%d\t%s\t%8d\t%s\t", curInode->i_links_count, timeBuf, curInode->i_size, permissions);
                printName(dp->name, dp->name_len);
                printf("\n");
            }
        }

        //TODO implement singly and doubly indirect blocks
    }
}

#define FUNCTIONSCOUNT 5

char *functionNames[FUNCTIONSCOUNT] = {"ls", "cd", "cat", "help", "pwd"};

void help(){

    printf("Available commands are:\n");
    for (int i = 0; i < FUNCTIONSCOUNT; ++i) {
        printf("%s\n", functionNames[i]);
    }
}

void (*functions[FUNCTIONSCOUNT])() = {ls, cd, cat, help, pwd};

int main(int argc, char* args[]) {
    while(filep == 0) {
        char fileName[124];
        int i = 0;

        if (argc > 1) {
            strcpy(fileName, args[1]);
        } else {
            printf("disk mount location: ");
            scanf("%s", fileName);
        }

        filep = open(fileName, O_RDONLY);

        if(filep == 0){
            printf("Failed to open disk, try again.\n");
        }

        init(filep);

        if(sp->s_magic != 0xEF53){
            printf("Not EXT2, it was %d", sp->s_magic);
            exit(1);
        } else {

            printf("SUPER\tmagic=%x\tbmap=%d\timap=%d\tiblock=%d\n", sp->s_magic, gp->bg_block_bitmap, gp->bg_inode_bitmap, gp->bg_inode_table);
        }
    }

    char response[1024] = "\0";

    do {
        printf("$ ");
        fgets(&response, 1024, stdin);

        char *c = strchr(response, '\n');
        if(c) {
            *c = '\0';
        }

        char *arguments = strtok(response, " ");
        arguments = strtok(NULL, "\0");

        int commandFound = 0;

        for(int i = 0; i < FUNCTIONSCOUNT; i++){
            if(strcmp(functionNames[i], response) == 0){
                functions[i](arguments);
                commandFound = 1;
                break;
            }
        }

        if(!commandFound){
            printf("Command %s was not found\n", response);
        }

    } while (strcmp(response, "quit") != 0);

}


