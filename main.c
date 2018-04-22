//
// Created by whomagoo on 4/13/18.
//

#include <stdio.h>

#include "disk.h"

int main(int argc, char* args[]) {


    int file = 0;

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
        SUPER *sp = getSp();
        GD *gd = getGd();

        if(sp->s_magic != 0xEF53){
            printf("Not EXT2, it was %d", sp->s_magic);
            exit(1);
        } else {

            printf("SUPER\tmagic=%x\tbmap=%d\timap=%d\tiblock=%d", sp->s_magic, gd->bg_block_bitmap, gd->bg_inode_bitmap, gd->bg_inode_table);
        }
    }





}