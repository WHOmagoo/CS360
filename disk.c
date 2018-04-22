#include "disk.h"

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

char gBuf[BLKSIZE];
char sBuf[BLKSIZE];
char iBuf[BLKSIZE];
char dBuf[BLKSIZE];

int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd,(long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int init(int fd){
    get_block(fd, 1, sBuf);
    sp = (SUPER *) sBuf;

    get_block(fd, 2, gBuf);
    gp = (GD *) gBuf;
}

SUPER * getSp(){
    return sp;
}

GD * getGd(){
    return gp;
}

INODE * getIp(){
    return ip;
}

DIR * getDp(){
    return dp;
}
