#include "disk.h"

GD    *gp;
SUPER *sp;

//ip is the current iNode
INODE *ip;

//dp is the current directory
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

    ip = (INODE *) iBuf + 1;
}

int startingIblock(){
    return gp->bg_inode_table;
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
