//
// Created by whomagoo on 4/13/18.
//

#include <stdio.h>

int main(char* args[], int argc) {


    FILE *file = 0;

    while(file == 0) {
        char fileName[124];
        int i = 0;

        if (argc > 0) {
            strcpy(fileName, args[0]);
        } else {
            printf("disk mount location: ");
            scanf("%s", fileName);
        }

        file = fopen(fileName, "rw");

        if(file == 0){
            printf("Failed to open disk, try again.\n");
        }
    }



}