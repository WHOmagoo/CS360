//
// Created by whomagoo on 2/14/18.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char home[64];
char curDirectory[128];

struct PathList *path;

typedef struct PathList{
    char path[64];
    struct PathList *next;
}PathList;

int main(int argc, char *argv[], char *env[]){
    for(int i = 0; env[i]; i++){
        printf("%s\n", env[i]);
        char envProp[strlen(env[i])+1];
        strcpy(envProp, env[i]);


        char *token = strtok(envProp, "=");

        if(strcmp(token, "HOME") == 0){
            strcpy(home, strtok(NULL, "\0"));
        } else if (strcmp(token, "PATH") == 0){
            token = strtok(NULL, ":");
            int n = 0;
            PathList *prev = 0;
            while(token){
                PathList *curPath = (PathList *) malloc(sizeof(PathList));
                if(prev){
                    prev->next = curPath;
                } else {
                    path = curPath;
                }


                strcpy(curPath->path, token);
                curPath->next = 0;

                prev = curPath;
                token = strtok(NULL, ":");
            }
        }
    }

    PathList *tmp = path;

    while(tmp){
        printf("%s\n", tmp->path);
        tmp = tmp->next;
    }

    printf("HOME = %s\n", home);
    strcpy(curDirectory, home);

    char command[128] = "\0";

    while(strcmp(command, "exit") != 0) {
        printf("%s $ ", curDirectory);
        scanf("%127s", command);
        execute(command, env);
    }
}

int execute(char *program, char *env[]){
    PathList *cur = path;
    char *myargv[3] = {NULL,  "TestBoi", NULL};

    while(cur){
        int result = 0;

        char tmp[128] = "\0";
        strcpy(tmp, cur->path);
        strcat(tmp, "/");
        strcat(tmp, program);

        myargv[0] = tmp;

        printf("%s", myargv[0]);
        result = execve(myargv[0], myargv, env);

        if(result == -1){
            printf("Not found in %s\n", cur->path);
        }

        cur = cur->next;

    }
}