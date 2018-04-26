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
#include <math.h>

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
    return read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[])
{
//    printf("Writing block %d\n", blk);
//    if(blk == 14){
//        INODE *tmps = (INODE *) buf;
//        printf("Inode 14 points to %d\n", tmps->i_block[0]);
//    } else if (blk == 33){
//        INODE *tmps = (INODE *) buf;
//        printf("Inode 33 points to %d\n", tmps->i_block[0]);
//    }

    lseek(fd, (long)blk*BLKSIZE, 0);
    write(fd, buf, BLKSIZE);
    fsync(fd);
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

                if(type == -1){
                    bufcpy(buf, buftmp);
                    curDir = (INODE *) ((__uint32_t) buf + (u_int32_t) dirTmp - (u_int32_t) buftmp);
                    return curDir;
                } else if(dp->file_type == type || ((dirTmp->i_mode >> 14) % 2 == 1 && type == 2)) {
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

//use -1 for any inode type
INODE * getInode(char *dir, char buf[BLKSIZE], int type){
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
        INODE * nextDir = getInBlock(lookingFor, curBuf, curDir, type);
        if(nextDir) {
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

INODE * getDir(char *dir, char buf[BLKSIZE]){
    return getInode(dir, buf, 2);
}


//returns a pointer to the part after the last '/', if there are none or there is one , it returns 0;
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

//returns the inode number of the current inode
int getParent(INODE **inode, char buf[BLKSIZE]){
    int block = (*inode)->i_block[0];
    int result = get_block(filep, (*inode)->i_block[0], buf);

//    printf("Result of getting block %d was:%d\n", block, result);


    DIR *parentRef = (DIR *) buf;
    int curInode = parentRef->inode;
    parentRef = (DIR *) (buf + parentRef->rec_len);

    int parentInode = parentRef->inode;

    get_block(filep, gp->bg_inode_table + (parentInode - 1) / 8, buf);
    (*inode) = (INODE *) buf + (parentInode - 1) % 8;

//    printf("Parent looks in block %d\n", (*inode)->i_block[0]);

    return curInode;
}

int getInodeNum(INODE *inode, char buf[BLKSIZE]){
    char thisBuf[BLKSIZE];
    bufcpy(thisBuf, buf);
    INODE *thisInode = (INODE *) ((u_int32_t) thisBuf + (u_int32_t) inode - (u_int32_t) buf);
    return getParent(&thisInode, thisBuf);
}

char *getName(INODE *inode, char buf[BLKSIZE]){
    int iNodeNum = getParent(&inode, buf);
//    printf("inodenum %d\n", iNodeNum);
    //inode = (INODE *) buf;

//    printf("GP inode table = %d", gp->bg_inode_table);

    int block = gp->bg_inode_table + (iNodeNum - 1) / 8;
    int offset = (iNodeNum - 1) % 8;

//    printf("Block: %d, offset: %d" ,block, offset);

    int blocks[15];
    for(int i = 0; i < 15; i++){
        blocks[i] = inode->i_block[i];
        if(blocks[i] == 0){
            break;
        }
    }

    for(int i = 0; i < 12; i++){
        if(blocks[i] == 0){
            break;
        }

        get_block(filep, blocks[i], buf);
        int len = 0;
        while (len < BLKSIZE) {
            DIR *dp = (DIR *) (buf + len);
            len += dp->rec_len;

            if(dp->inode == iNodeNum){
                *(dp->name + dp->name_len) = '\0';
                return dp->name;
            }
        }
    }

    printf("Unable to find name\n");
    return 0;

    //TODO implement singly and doubly indirect blocks

}

void pwd(){

    //printf("%d\n", cwd->i_block[0]);
    char curBuf[BLKSIZE];
    char parentBuf[BLKSIZE];

    bufcpy(curBuf, iBuf);
    bufcpy(parentBuf, curBuf);

    INODE *cur = (INODE *) ((u_int32_t ) curBuf + (u_int32_t) cwd - (u_int32_t) iBuf);
    INODE *parent = (INODE *) ((u_int32_t ) parentBuf + (u_int32_t) cwd - (u_int32_t) iBuf);

    char wd[BLKSIZE] = "";
    char cd[BLKSIZE] = "";

    while(1) {
        bufcpy(curBuf, parentBuf);
        cur = (INODE *) ((u_int32_t ) curBuf + (u_int32_t) parent - (u_int32_t) parentBuf);

        char *name = getName(cur, curBuf);
        strcpy(cd, name);

        if(strcmp(".", cd) == 0){
            cd[0] = '/';
            cd[1] = '\0';


            strcat(cd, wd);
            strcpy(wd, cd);

            break;
        }


        strcat(cd, "/");
        strcat(cd, wd);
        strcpy(wd, cd);
        getParent(&parent, parentBuf);
    }

    printf("%s ", wd);

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

                int blockNum = gp->bg_inode_table + (dp->inode - 1) / 8;

                get_block(filep, blockNum, curItemBuf);

                curInode = (INODE *) curItemBuf + (dp->inode - 1) % 8;

                char timeBuf[80];
                getTime(curInode->i_atime, timeBuf);


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
                        break;
                }

                printf("\t%d\t%s\t%8d\t%s\t%d\t%d\t%d\t", curInode->i_links_count, timeBuf, curInode->i_size, permissions, curInode->i_block[0], dp->inode, blockNum);
                printName(dp->name, dp->name_len);
                printf("\n");
            }
        }

        //TODO implement singly and doubly indirect blocks
    }
}

int decFreeInodes(int dev)
{

    // dec free inodes count in SUPER and GD
    get_block(dev, 1, sBuf);
    sp->s_free_inodes_count--;
    put_block(dev, 1, sBuf);

    get_block(dev, 2, gBuf);
    gp->bg_free_inodes_count--;
    put_block(dev, 2, gBuf);
}

int incFreeInodes(int file){
    get_block(file, 1, sBuf);
    sp->s_free_inodes_count++;
    put_block(file, 1, sBuf);

    get_block(file, 2, gBuf);
    gp->bg_free_inodes_count++;
    put_block(file, 2, gBuf);
}

int tst_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    if (buf[i] & (1 << j))
        return 1;
    return 0;
}

int set_bit(char *buf, int bit)
{
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] |= (1 << j);
}

int unset_bit(char *buf, int bit){
    int i, j;
    i = bit/8; j=bit%8;
    buf[i] = ~buf[i];
    buf[i]|= (1 << j);
    buf[i] = ~buf[i];
}

int destroyDiskBlock(int block){
    char buf[BLKSIZE];

    int bmap = gp->bg_block_bitmap;
    get_block(filep, bmap, buf);

    if(tst_bit(buf, block - 1)){
        unset_bit(buf, block - 1);
        int unset = tst_bit(buf, block - 1);
        printf("Deleting block %d\n", block);
        put_block(filep, gp->bg_block_bitmap, buf);
    } else {
        printf("Trying to delete block %d but it was already deleted\n", block);
    }
}

int makeDiskBlock(){
    char buf[BLKSIZE];

    int nBlocks = sp->s_blocks_count;

    //printf("Searching through %d to allocate a block\n", gp->bg_block_bitmap);
    int bmap = gp->bg_block_bitmap;

    get_block(filep, bmap, buf);

    for (int i=0; i < BLKSIZE; i++){
        if(!tst_bit(buf, i)){
            int oldBit = tst_bit(buf, i);
            set_bit(buf, i);
            int newBit = tst_bit(buf, i);

            //printf("Old: %d, New: %d for i = %d\n", oldBit, newBit, i);

            put_block(filep, bmap, buf);

            printf("Allocating to block %d", i + 1);
            return i + 1;
        }
    }

    printf("No more space to allocate a block");
    return -1;
}

int ialloc(int dev)
{
    int  i;
    char buf[BLKSIZE];

    //printf("Searching through %d to allocate an inode", gp->bg_inode_bitmap);
    get_block(dev, gp->bg_inode_bitmap, buf);

    //printf("Inodes count %d", sp->s_inodes_count);

    for (i=0; i < sp->s_inodes_count; i++){
        if (tst_bit(buf, i)==0){

            printf("Allocating to inode %d\n", i + 1);
            set_bit(buf,i);
            decFreeInodes(dev);

            put_block(dev, gp->bg_inode_bitmap, buf);

            return i+1;
        }
    }

    printf("ialloc(): no more free inodes\n");
    return 0;
}

int idealloc(int file, int inodeNum){
    printf("Deallocating inode %d\n", inodeNum);
    inodeNum -= 1;
    char buf[BLKSIZE];

    get_block(file, gp->bg_inode_bitmap, buf);

    if(tst_bit(buf, inodeNum) == 1){
        unset_bit(buf, inodeNum);
        incFreeInodes(file);

        put_block(file, gp->bg_inode_bitmap, buf);
        return 1;
    } else {
        printf("Inode %s was already deallocated\n", inodeNum);
        return 0;
    }
}

void my_rmdir(char* pathName){
    char buf[BLKSIZE];
    char parentBuf[BLKSIZE];

    int curInode = getInodeNum(cwd, iBuf);

    INODE *toDelete = getDir(pathName, buf);

    if(toDelete) {
        bufcpy(parentBuf, buf);
        INODE *parent = (INODE *) ((u_int32_t) parentBuf + (u_int32_t) toDelete - (u_int32_t) buf);
        int inodeToRemove = getParent(&parent, parentBuf);

        int blocks[15];

        for (int i = 0; i < 15; i++) {
            blocks[i] = toDelete->i_block[i];
            if (blocks[i] == 0) {
                break;
            }
        }

        int offset = 0;
        get_block(filep, blocks[0], buf);
        DIR *dp = (DIR *) buf;
        offset += dp->rec_len;
        dp = (DIR *) (buf + offset);
        offset += dp->rec_len;


        if (offset < BLKSIZE || blocks[1] != 0) {
            printf("Couldn't remove %s, it was non empty", pathName);
            return;
        }

        if (curInode == ((DIR *) buf)->inode) {
            printf("Couldn't remove %s, it was the current directory", pathName);
            return;
        }

        int parentBlocks[15];

        for (int i = 0; i < 15; i++) {
            parentBlocks[i] = parent->i_block[i];
        }

        for (int i = 0; i < 12; i++) {
            char dirBuf[BLKSIZE];
            get_block(filep, parentBlocks[i], dirBuf);

            int len = 0;


            DIR *prev = 0, *dp = (DIR *) dirBuf;

            while (len < BLKSIZE) {

                if (dp->inode == inodeToRemove) {
                    if (!prev) {
                        if (i > 0) {
                            //This will bug if the deleted directory has a name length close to BLKSIZE
                            idealloc(filep, parentBlocks[i]);
                            parent->i_block[i] = 0;
                            put_block(filep, gp->bg_inode_table + (getInodeNum(parent, buf) - 1) / 8, buf);

                            //break out of for loop and while loop;
                            i = 69;
                            break;
                        } else {
                            printf("Error, trying to remove root\n");
                            return;
                        }
                    }

                    if (dp->file_type != 2) {
                        printf("Trying to delete something that wasn't a directory\n");
                        return;
                    }

                    prev->rec_len += dp->rec_len;
                    put_block(filep, parentBlocks[i], dirBuf);
                    //break out of while and for loop
                    i = 69;
                    break;

                }

                prev = dp;
                len += dp->rec_len;
                dp = (DIR *) (dirBuf + len);
            }
        }

        //remove the blocks that the folder was using
        for (int i = 0; i < 12; i++) {
            if (blocks[i] == 0) {
                break;
            }
            destroyDiskBlock(blocks[i]);
        }

        idealloc(filep, inodeToRemove);
        //shouldn't need to look in the singly and doubly linked blocks because they should be 0
    }
}

void mkdir(char* pathName){
    if(pathName == 0){
        printf("No path specified");
        return;
    }

    //remove trailing back slash
    pathName = strtok(pathName, "\n");

    char buf[BLKSIZE];
    char *baseName = splitLast(pathName);

    if(baseName == 0) {
        if(*pathName == '/'){
            baseName = pathName + 1;
            pathName = "/";
        } else {
            baseName = pathName;
            pathName = 0;
        }
    }

    INODE *cur = getDir(pathName, buf);

    if(cur) {
        int blocks[15];
        int newItemLen = strlen(baseName);
        int rec_len = newItemLen + ((newItemLen - 1) % 4 - 4) * -1 - 1 + 8;

        for (int i = 0; i < 15; i++) {
            blocks[i] = cur->i_block[i];
        }

        for (int i = 0; i < 15; i++) {
            get_block(filep, blocks[i], buf);
            int len = 0;

            while (len < BLKSIZE) {
                DIR *dp = (DIR *) (buf + len);
                len += dp->rec_len;

                if (my_strcmp(dp->name, baseName, dp->name_len) == 0) {
                    if (!pathName) {
                        pathName = "directory";
                    }
                    printf("Could not make %s, it already exists in %s", baseName, pathName);
                    return;
                }

                if (blocks[i] == 0) {
                    break;
                }
            }
        }

        get_block(filep, blocks[0], buf);

        DIR *tmp = (DIR *) buf;
        __u32 parentInode = tmp->inode;


        int success = 0;
        for (int i = 0; i < 12; i++) {
            DIR *dp;



            int len = 0;

            if(blocks[i] == 0){
                blocks[i] = makeDiskBlock();

                printf("We might get errors\n");
                //TODO inode->blocks doesn't get incremented
                get_block(filep, blocks[i], buf);
                dp = (DIR *) buf;
                dp->name_len = 0;
                dp->rec_len = BLKSIZE + 8;
            } else {
                get_block(filep, blocks[i], buf);
            }

            while (len < BLKSIZE) {
                dp = (DIR *) (buf + len);

                int distanceTo4 = ((dp->name_len - 1) % 4 - 4) * -1 - 1;
                __u16 excessSpace; // = (__u16) (dp->rec_len - dp->name_len + 4 - (dp->name_len % 4) - 8);
                excessSpace = (__u16) (dp->rec_len - 8 - dp->name_len - distanceTo4);
                if (excessSpace >= rec_len) {


                    __u16 new_rec_len = (__u16) (dp->name_len + ((dp->name_len - 1) % 4 - 4) * -1 - 1 + 8);
                    dp->rec_len = new_rec_len;

                    len += dp->rec_len;

                    dp = (DIR *) (buf + len);
                    dp->name_len = (__u8) newItemLen;
                    dp->rec_len = excessSpace;
                    strcpy(dp->name, baseName);
                    dp->file_type = 2;

                    __u32 inode = (__u32) ialloc(filep);
                    dp->inode = inode;
                    //printf("Allocated to inode %d\n", inode);

                    put_block(filep, blocks[i], buf);

                    get_block(filep, gp->bg_inode_table + (inode - 1) / 8, buf);
                    INODE *newDir = (INODE *) buf + (inode - 1) % 8;
                    newDir->i_mode = 16877;
                    __u32 block = (__u32) makeDiskBlock();
                    newDir->i_block[0] = block;
                    newDir->i_block[1] = 0;
                    newDir->i_ctime = (__u32) time(NULL);
                    newDir->i_mtime = newDir->i_ctime;
                    newDir->i_atime = newDir->i_ctime;
                    newDir->i_links_count = 1;
                    newDir->i_flags = 0;
                    newDir->i_size = 1024;

                    put_block(filep, gp->bg_inode_table + (inode - 1) / 8, buf);

                    get_block(filep, block, buf);

                    dp = (DIR *) buf;
                    dp->inode = inode;
                    strcpy(dp->name, ".");
                    dp->rec_len = 12;
                    dp->name_len = 1;
                    dp->file_type = 2;

                    dp = (DIR *) (buf + 12);
                    dp->inode = parentInode;
                    strcpy(dp->name, "..");
                    dp->rec_len = BLKSIZE - 12;
                    dp->file_type = 2;
                    dp->name_len = 2;

                    dp = (DIR *) buf;
                    dp = (DIR *) (buf + 12);

                    put_block(filep, block, buf);





                    //break out of for loop and while loop
                    success = 1;
                    i = 69;
                    break;
                }

                len += dp->rec_len;

            }

        }

        if (!success) {
            //TODO allocate a new inode and put it in the wd i_block

            //TODO \/

        }

        //Reload the current directory in case cwd has been modified
        char reload[4] = ".";
        cd(reload);
    } else {
        printf("Could not find directory");
    }

}

void touch(char * dir){
    if(dir) {
        char pathName[2];
        char *basename = splitLast(dir);
        if (!basename && *dir != '/') {
            basename = dir;
            pathName[0] = '/';
            pathName[1] = '\0';
            dir = pathName;
        } else if (!basename && *dir == '/'){
            basename = dir + 1;
            pathName[0] = '/';
            pathName[1] = '\0';
            dir = pathName;
        }

        char buf[BLKSIZE];
        char backup[BLKSIZE];
        INODE *parentDirectory = getDir(dir, buf);

        if(parentDirectory) {
            INODE *toTouch = getInBlock(basename, buf, parentDirectory, -1);
            if(toTouch) {
                toTouch->i_atime = (__u32) time(NULL);
                toTouch->i_mtime = toTouch->i_atime;
                bufcpy(backup, buf);
                put_block(filep, gp->bg_inode_table + (getParent(&toTouch, buf) - 1) / 8, backup);
            } else {
                return;
            }
        } else {
            return;
        }

    } else {
        printf("No file specified");
    }
}

void stat(char *dir){
    if(dir) {
        char pathname[2];
        char *basename = splitLast(dir);
        if(!basename && *dir != '/'){
            basename = dir;
            pathname[0] = '/';
            pathname[1] = '\0';
        } else if(!basename && *dir == '/'){
            basename = dir+1;
            pathname[0] = '/';
            pathname[1] = '\0';
        }

        char buf[BLKSIZE];

        INODE *item = getInode(dir, buf, -1);

        int inode = -1;
        int mode = item->i_mode;
        int uid = item->i_uid;
        int gid = item->i_gid;
        int nlink =item->i_links_count;
        int size = item->i_size;

        char cBuf[80];
        int ctime = item->i_ctime;
        getTime(ctime, cBuf);

        char mBuf[80];
        int mtime = item->i_mtime;
        getTime(mtime, mBuf);

        char aBuf[80];
        int atime = item->i_atime;
        getTime(atime, aBuf);

        inode = getParent(&item, buf);

        printf("inode %d\tmode %o\tuid %d\n", inode, mode, uid);
        printf("gid %d\tnlink %d\tsize %d\n", gid, nlink, size);
        printf("Created: %s\n", cBuf);
        printf("Accessed: %s\n", aBuf);
        printf("Modified: %s\n", mBuf);

    }
}

#define FUNCTIONSCOUNT 9

char *functionNames[FUNCTIONSCOUNT] = {"ls", "cd", "cat", "help", "pwd", "mkdir", "rmdir", "touch", "stat"};

void help(){

    printf("Available commands are:\n");
    for (int i = 0; i < FUNCTIONSCOUNT; ++i) {
        printf("%s\n", functionNames[i]);
    }
}

void (*functions[FUNCTIONSCOUNT])() = {ls, cd, cat, help, pwd, mkdir, my_rmdir, touch, stat};

int main(int argc, char* args[]) {

//
//    for(int size = 0;size < 15; size++) {
//        int distanceUpTo4 = ((size - 1) % 4 - 4) * -1 - 1;
//        int result = size + distanceUpTo4;
//        printf("Size = %d, result = %d, distance = %d\n",size, result, distanceUpTo4);
//    }



    while(filep == 0) {
        char fileName[124];
        int i = 0;

        if (argc > 1) {
            strcpy(fileName, args[1]);
        } else {
            printf("disk mount location: ");
            scanf("%s", fileName);
        }

        filep = open(fileName, O_RDWR);

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
        pwd();
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
                printf("\n");
                commandFound = 1;
                break;
            }
        }

        if(!commandFound){
            printf("Command %s was not found\n", response);
        }

    } while (strcmp(response, "quit") != 0);

    //TODO implement unmount

}


