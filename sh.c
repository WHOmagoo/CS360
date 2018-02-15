//
// Created by whomagoo on 2/14/18.
//

#include <stdio.h>

int main(int argc, const char *argv[]){
    printf("YO\n");
    for (int i = 0; i < argc; ++i) {
        printf("%d: %s\n", i, argv[i]);
    }

    printf("after the loop");
}