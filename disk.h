//
// Created by whomagoo on 4/13/18.
//

#ifndef SHSIMULATOR_DISK_H
#define SHSIMULATOR_DISK_H

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

int get_block(int fd, int blk, char buf[ ]);
int initSuper(int fd);
int startingIblock();
SUPER * getSp();
GD * getGd();
INODE * getIp();
DIR * getDir();

#endif //SHSIMULATOR_DISK_H
