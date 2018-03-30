/********* inode.c: print information in / INODE (INODE #2) *********/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <memory.h>

#define BLKSIZE 1024

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

int fd;
int iblock;

int get_block(int fd, int blk, char buf[ ])
{
    lseek(fd,(long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

inode()
{
    char buf[BLKSIZE];

    get_block(fd, 1, buf);
    sp = (SUPER *) buf;

    // read GD
    get_block(fd, 2, buf);
    gp = (GD *)buf;
    /****************
    printf("%8d %8d %8d %8d %8d %8d\n",
       gp->bg_block_bitmap,
       gp->bg_inode_bitmap,
       gp->bg_inode_table,
       gp->bg_free_blocks_count,
       gp->bg_free_inodes_count,
       gp->bg_used_dirs_count);
    ****************/
    iblock = gp->bg_inode_table;   // get inode start block#
    printf("inode_block=%d\n", iblock);

    // get inode start block
    get_block(fd, iblock, buf);

    ip = (INODE *)buf + 1;         // ip points at 2nd INODE

    printf("mode=%4x ", ip->i_mode);
    printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
    printf("size=%d\n", ip->i_size);
    printf("time=%s", ctime(&ip->i_ctime));
    printf("link=%d\n", ip->i_links_count);
    printf("i_block[0]=%d\n", ip->i_block[0]);

    /*****************************
     u16  i_mode;        // same as st_imode in stat() syscall
     u16  i_uid;                       // ownerID
     u32  i_size;                      // file size in bytes
     u32  i_atime;                     // time fields
     u32  i_ctime;
     u32  i_mtime;
     u32  i_dtime;
     u16  i_gid;                       // groupID
     u16  i_links_count;               // link count
     u32  i_blocks;                    // IGNORE
     u32  i_flags;                     // IGNORE
     u32  i_reserved1;                 // IGNORE
     u32  i_block[15];                 // IMPORTANT, but later
    ***************************/

    char file[124] = "X/tiny";

    lookUp(file);
}

lookUp(char *file){
    char *curEnd = strchr(file, '/');
    if(curEnd) {
        char buf[BLKSIZE];
        int blocks[15];
        for (int i = 0; i < 15; i++) {
            blocks[i] = ip->i_block[i];
        }
        get_block(fd, blocks[0], buf);

        *curEnd = '\0';
        int len = 0;


        while (len < BLKSIZE) {


            dp = (DIR *) (buf + len);

            printf("%s\n", dp->name);
            if (strcmp(dp->name, file) == 0) {
                printf("in %s, it is a %d type:\n", file, dp->file_type);
                int cur_inode = dp->inode;
                get_block(fd, iblock + (cur_inode - 1) / 8, buf);
                ip = (INODE *) buf + (cur_inode -1 ) % 8;
                lookUp(curEnd + 1);
                break;
            }
            len += dp->rec_len;
        }
    } else {
        printf("Reached end of the path, looking for '%s'\n", file);
        int blocks[15];
        for(int i = 0; i < 15; i++){
            blocks[i] = ip->i_block[i];
        }
        char buf[BLKSIZE];

        get_block(fd, blocks[0], buf);

        int len = 0;

        while (len < BLKSIZE) {


            dp = (DIR *) (buf + len);

            printf("%s\n", dp->name);
            if (strcmp(dp->name, file) == 0) {
                printf("Found %s, it is a %d type:\n", file, dp->file_type);
                int cur_inode = dp->inode;
                get_block(fd, iblock + (cur_inode - 1) / 8, buf);
                ip = (INODE *) buf + (cur_inode -1 ) % 8;

                for(int i = 0; i < 15; i++) {
                    if(ip->i_block[i] == 0){
                        break;
                    }
                    printf("block[%2d] = %d\n", i, ip->i_block[i]);
                }

                break;
            }
            len += dp->rec_len;
        }

        return 0;
    }
}

char *disk = "mydisk";
main(int argc, char *argv[]) {
    if (argc > 1)
        disk = argv[1];

    fd = open(disk, O_RDONLY);
    if (fd < 0) {
        printf("open %s failed\n", disk);
        exit(1);
    }

    inode();
}