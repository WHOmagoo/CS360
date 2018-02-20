//
// Created by whomagoo on 2/14/18.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include "t.h"

struct PathList *path;

typedef struct PathList{
    char path[64];
    struct PathList *next;
}PathList;

int execute(char *args[], char *env[]){
    PathList *cur = path;

    int pid = fork();
    int status;
    if(pid){
        pid = wait(&status);
        printf("PID: %d, Status: %d\n", pid, status);
        return 1;
    } else {
        char programName[128];
        strcpy(programName, args[0]);

        int mode = 0;
        char *location = 0;
        char *newArgs;

        for (int i = 0; args[i]; ++i) {
            if(strcmp("<", args[i]) == 0){ //infile
                args[i] = NULL;
                i++;
                if(args[i]){
                    location = args[i];
                    mode = 1;
                    //close(0);

                    //open(args[i], O_RDONLY | O_CREAT, 0644);
                } else {
                    printf("Error, no file speicfied");
                    return -1;
                }
            } else if(strcmp(">", args[i]) == 0) { //outfile
                args[i] = NULL;
                i++;
                if(args[i]){
                    mode = 2;
                    location = args[i];
//                    close(1);
//                    open(param, O_CREAT | O_WRONLY, 0644);
                } else {
                    printf("Error, no file speicfied");
                    return -1;
                }
            } else if(strcmp(">>", args[i]) == 0){ //append
                args[i] = NULL;
                i++;
                if(args[i]){
                    mode = 3;
                    location = args[i];
//                    close(1);
//                    printf("Opening %s in append mode\n", param);
//                    open(param,  O_WRONLY | O_APPEND | O_CREAT, 0644);
                } else {
                    printf("Error, no file specified");
                    return -1;
                }
            } else if(strcmp("|", args[i]) == 0){
                args[i] == NULL;
                newArgs = args[i+1];
                mode = 4;
            }

        }


        int stdIn = dup(0);
        int stdOut = dup(1);

        while (cur) {
            int result = 0;

            char tmp[128] = "\0";
            strcpy(tmp, cur->path);
            strcat(tmp, "/");
            strcat(tmp, programName);

            args[0] = tmp;

            if(mode == 1){
                close(0);

                open(location, O_RDONLY | O_CREAT, 0644);
            } else if (mode == 2){
                close(1);

                open(location, O_WRONLY | O_CREAT, 0644);
            } else if (mode == 3){
                close(1);

                open(location, O_WRONLY | O_APPEND | O_CREAT, 0644);
            }


            if(mode == 4){
                int pd[2];
                pipe(pd);
                pid = fork();
                if (pid){ // parent as pipe WRITER
                    close(pd[0]);
                    close(1);
                    dup(pd[1]);
                    close(pd[1]);
                    execve(args[0], args, env);
                }
                else{     // child as pipe READER
                    close(pd[1]);
                    close(0);
                    dup(pd[0]);
                    close(pd[0]);
                    execve(newArgs[0], newArgs, env);
                }
            } else {
                result = execve(args[0], args, env);
            }
            if (result == -1) {
                if(location && (mode == 2 || mode == 3)) {
                    close(location);
                    dup2(stdOut, 1);
                }
                printf("not found at %s\n", args[0]);
            }

            cur = cur->next;


        }
    }
}

int main(int argc, char *argv[], char *env[]){

//    init();
//    kfork();

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


    while(1) {
        char command[128] = "\0";
        printf("%s $ ", pwd + 4);

        scanf("%127[^\n]", command);
        char newline;
        scanf("%c", &newline); //consume trailing newline character


        PathList *prev = 0, *firstParam = 0;
        int paramCount = 0;

        char *param = strtok(command, " ");

        while (param) {

            PathList *cur = (PathList *) malloc(sizeof(PathList));
            paramCount++;
            strcpy(cur->path, param);
            cur->next = 0;
            if (prev) {
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
            PathList *last = prev;
            prev = prev->next;
            free(last);
        }


    }

}