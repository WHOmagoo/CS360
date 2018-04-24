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

int file = 0;

GD    *gp;
SUPER *sp;

//cwd is the current iNode or cwd and should point at iBuf
INODE *cwd;

char gBuf[BLKSIZE];
char sBuf[BLKSIZE];
char iBuf[BLKSIZE];
char dBuf[BLKSIZE];

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

    return 0;
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

INODE * getDir(char *dir, char buf[]){
    char curBuf[BLKSIZE];
    INODE *curDir;

    if(dir == 0){
        bufcpy(buf, iBuf);
        return (INODE *) ((u_int32_t )buf + (u_int32_t )cwd - (u_int32_t )iBuf);
    }

    if(*dir == '/'){
        dir++;
        //copy root into curBuf
        get_block(file, gp->bg_inode_table, curBuf);
        curDir = (INODE *) curBuf + 1;
    } else {
        //copy cwd into curBuf
        bufcpy(curBuf, iBuf);
        curDir = (INODE *) ((u_int32_t ) curBuf + (u_int32_t ) cwd - (u_int32_t ) iBuf);
    }


    char *lookingFor = strtok(dir, "/");
    int finished = 0;

    while (lookingFor != 0) {

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

            get_block(file, curBlock[i], nextDir);

            int len = 0;

            while (len < BLKSIZE) {
                DIR *dp = (DIR *) (nextDir + len);
                char *name = dp->name;

                if (my_strcmp(dp->name, dir, dp->name_len) == 0) {

                    get_block(file, gp->bg_inode_table + (dp->inode - 1) / 8, curBuf);
                    curDir = (INODE *) curBuf + (dp->inode - 1) % 8;

                    if (curDir->i_mode >> 14 % 2 || dp->file_type == 2) {
                        //break out of the for loop and the while loop
                        found = 1;
                        break;
                    } else {
                        //break out of the for loop and the while loop
                        //found the next node but it wasn't a directory
                        printf("%s was not a directory. ", lookingFor);
                        i = 69;
                        break;
                    }


                }
                len += dp->rec_len;
            }
        }

        //TODO implement singly and doubly indirect blocks
        
        if(!found){
            printf("Couldn't find %s\n", lookingFor);
            return 0;
        }
        
        
        lookingFor = strtok(NULL, "/");
        
    }


    bufcpy(buf, curBuf);
    return (INODE *) ((u_int32_t )buf + (u_int32_t )curDir - (u_int32_t ) curBuf);


}

void cd(char *dir){
    if(dir == 0){
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

        for (int i = 0; i < 12; i++) {
            int curBlock = curInode->i_block[i];
            char buf[BLKSIZE];

            if (curBlock == 0) {
                break;
            }

            get_block(file, curBlock, buf);

            int len = 0;

            while (len < BLKSIZE) {
                DIR *dp = (DIR *) (buf + len);
                len += dp->rec_len;

                char name[BLKSIZE];

                strcpy(name, dp->name);

                char curItemBuf[BLKSIZE];
                get_block(file, gp->bg_inode_table + (dp->inode - 1) / 8, curItemBuf);

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

#define FUNCTIONSCOUNT 2

void (*functions[FUNCTIONSCOUNT])() = {ls, cd};

char *functionNames[FUNCTIONSCOUNT] = {"ls", "cd"};

int main(int argc, char* args[]) {

    while(file == 0) {
        char fileName[124];
        int i = 0;

        if (argc > 1) {
            strcpy(fileName, args[1]);
        } else {
            printf("disk mount location: ");
            scanf("%s", fileName);
        }

        file = open(fileName, O_RDONLY);

        if(file == 0){
            printf("Failed to open disk, try again.\n");
        }

        init(file);

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

        for(int i = 0; i < FUNCTIONSCOUNT; i++){
            if(strcmp(functionNames[i], response) == 0){
                functions[i](arguments);
                break;
            }
        }


    } while (strcmp(response, "quit") != 0);

}


