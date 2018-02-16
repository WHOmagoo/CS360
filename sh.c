//
// Created by whomagoo on 2/14/18.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct PathList *path;

typedef struct PathList{
    char path[64];
    struct PathList *next;
}PathList;

int execute(char *args[], char *env[]){
    PathList *cur = path;

    char programName[128];
    strcpy(programName, args[0]);

    while(cur){
        int result = 0;

        char tmp[128] = "\0";
        strcpy(tmp, cur->path);
        strcat(tmp, "/");
        strcat(tmp, programName);

        args[0] = tmp;

        result = execve(args[0], args, env);

        if(result == -1){
            printf("not found at %s\n", args[0]);
        }

        cur = cur->next;

    }
}

int main(int argc, char *argv[], char *env[]){

    char home[128];
    int homeIndex = -1;
    char pwd[128];
    int pwdIndex = -1;

    for(int i = 0; env[i]; i++){
        printf("%s\n", env[i]);
        char envProp[strlen(env[i])+1];
        strcpy(envProp, env[i]);


        char *token = strtok(envProp, "=");

        if(strcmp(token, "HOME") == 0){
            strcpy(home, strtok(NULL, "\0"));
            homeIndex= i;
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
        } else if (strcmp(token, "PWD") == 0){
            strcpy(pwd, "PWD=");
            strcat(pwd, strtok(NULL, "\0"));
            pwdIndex = i;
        }
    }

    PathList *tmp = path;

    while(tmp){
        printf("%s\n", tmp->path);
        tmp = tmp->next;
    }

    printf("HOME = %s\n", home);

    char command[128] = "\0";

    while(1) {
        printf("%s $ ", pwd + 4);

        scanf("%127[^\n]", command);
        char newline;
        scanf("%c", &newline); //consume trailing newline character


        PathList *prev = 0, *firstParam = 0;
        int paramCount = 0;

        char *param = strtok(command, " ");

        while(param){
            PathList *cur = (PathList *) malloc(sizeof(PathList));
            paramCount++;
            strcpy(cur->path, param);
            cur->next = 0;
            if(prev){
                prev->next = cur;
            } else {
                firstParam = cur;
            }
            prev = cur;

            param = strtok(NULL, " ");
        }

        char *args[paramCount + 1];

        prev = firstParam;

        for(int count = 0; count < paramCount; count++){
            args[count] = firstParam->path;
            firstParam = firstParam->next;
        }

        args[paramCount] = 0;

        for (int i = 0; i <= paramCount; ++i) {
            printf("%s ", args[i]);
        }

        printf("\n");

        if(paramCount > 0){
            if(strcmp(args[0], "exit") == 0){
                exit(1);
            } else if(strcmp(args[0], "cd") == 0){
                if(args[1]) {
                    chdir(args[1]);
                    char *tmp;
                    getcwd(tmp, 128);
                    strcpy(pwd, "PWD=");
                    strcat(pwd, tmp);
                    env[pwdIndex] = pwd;
                } else {
                    chdir(home);
                    strcpy(pwd, "PWD=");
                    strcat(pwd, home);
                }

                env[pwdIndex] = pwd;
            } else {
                execute(args, env);
            }
        }


        while(prev){
            PathList *tmp = prev;
            prev = prev->next;
            free(tmp);
        }
    }
}