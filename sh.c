//
// Created by whomagoo on 2/14/18.
//

#include <stdio.h>
#include <string.h>

char home[64];
char curDirectory[128];

int main(int argc, char *argv[], char *env[]){
    for(int i = 0; env[i]; i++){
        char envProp[strlen(env[i])+1];
        strcpy(envProp, env[i]);
        char *token = strtok(envProp, "=");
        if(strcmp(token, "HOME") == 0){
            strcpy(home, strtok(NULL, "\0"));
        } else if (strcmp(token, "PATH") == 0){
            printf("Path = %s\n", strtok(NULL, "\0"));
        }
//        envProp = strtok(env[i], "=");
//        if(strcmp(envProp, "PATH") == 0){
//            strcpy(path, env[i]);
//            printf("%s", path);
//        }
    }

    printf("HOME = %s\n", home);
    strcpy(curDirectory, home);

    char command[128] = "\0";

    while(strcmp(command, "exit") != 0) {
        printf("%s $ ", curDirectory);
        scanf("%127s", command);
    }
}