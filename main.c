//
// Created by whomagoo on 1/25/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NODE{
    char name[64];
    char type;
    struct NODE *child, *sibling, *parent;
} NODE;

NODE *root, *cwd;
char line[128]; //User input line
char command[16], pathname[64]; //user inputs
char dirname[64], basename[64]; //string holders

int splitLast(){
    if(strcmp(pathname, "\0") != 0) {
        char *result, *previousResult = 0;
        strcpy(dirname, pathname);
        result = strtok(pathname, "/");

        while (result != NULL) {
            previousResult = result;
            result = strtok(NULL, "/");
        }

        //check if previousResult exists otherwise nothing was specified and set dirname and basename to nothing
        if(previousResult) {

            if (previousResult == pathname) {
                dirname[0] = '\0';
            } else {

                //trim trailing '/'
                dirname[(previousResult - pathname - 1) / sizeof(char)] = '\0';

                //re-add the '/' if that was the only directory specified
                if (strcmp(dirname, "\0") == 0) {
                    dirname[0] = '/';
                    dirname[1] = '\0';
                }


            }

            strcpy(basename, previousResult);
        } else {
            dirname[0] = '\0';
            basename[0] = '\0';
        }

        //debug code
        //printf("dirname: %s, basename:%s\n", dirname, basename);
    } else {
        //printf("dirname: \"\", basename: \"\"\n");
    }
}

NODE* makeChild(NODE *parent, char *name, char type){

    NODE *thisChild = parent->child, *lastChild = parent->child;

    if(name == 0 || *name == '\0'){
        char *typeString = type;
        if(type == 'D'){
            typeString = "directory";
        } else if (type == 'F'){
            typeString = "file";
        }
        printf("Could not make %s, no name was specified\n", typeString);
        return 0;
    }

    while(thisChild){
        if(strcmp(thisChild->name, name) == 0){
            printf("Could not make %s in %s. %s already exists in directory\n", name, parent->name, name);
            return 0;
        }

        lastChild = thisChild;
        thisChild = thisChild->sibling;
    }

    NODE* newNode = (NODE *) malloc(sizeof(NODE));
    strcpy(newNode->name, name);
    newNode->type = type;
    newNode->parent = parent;
    newNode->child = 0;
    newNode->sibling = 0;

    if(lastChild) {
        lastChild->sibling = newNode;
    } else {
        parent->child = newNode;
    }

    return newNode;

}

NODE* getSibling(NODE *child, char *name){
    if(child) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }

        return getSibling(child->sibling, name);
    } else {
        return 0;
    }
}

NODE* getNode(char nodeLocation[64], NODE* startingLocation){
    if(nodeLocation[0] == '/') {
        startingLocation = root;
        nodeLocation++;
    }

    char *curDirectory = strtok(nodeLocation, "/");

    while(curDirectory) {
        if (strcmp(curDirectory, "..") == 0) {
            startingLocation = startingLocation->parent;
        } else {
            NODE* tmp = getSibling(startingLocation->child, curDirectory);
            if(tmp == 0){
                printf("Search for directory %s in %s failed\n", curDirectory, startingLocation->name);
                return 0;
            } else {
                startingLocation = tmp;
            }
        }

        curDirectory = strtok(NULL, "/");
    }

    return startingLocation;
}

void mkdir(){
    splitLast();
    NODE *workingNode = getNode(dirname, cwd);
    if(workingNode) {
        if (workingNode->type == 'D') {
            makeChild(workingNode, basename, 'D');
        } else {
            printf("Could not insert folder to %s, it is a file\n", workingNode->name);
        }
    }
}

int isEmpty(NODE *node){
    return node->child == 0;
}

void rmdir(){
    NODE *toRemove = getNode(pathname, cwd);
    if(toRemove){
        if(toRemove->type == 'D'){
            removeNode(toRemove);
        } else {
            printf("Could not remove %s, it is NOT a directory\n", toRemove->name);
        }
    }
}

void creat(){
    splitLast();
    NODE *workingNode = getNode(dirname, cwd);
    if(workingNode) {
        if (workingNode->type == 'D') {
            makeChild(workingNode, basename, 'F');
        } else {
            printf("Could not insert file to %s, it is a file\n", workingNode->name);
        }
    }
}

NODE* getOlderSibling(NODE *youngerNode){
    NODE *curNode = youngerNode->parent->child;
    NODE *lastNode = 0;

    while(curNode && curNode != youngerNode){
        lastNode = curNode;
        curNode = curNode->sibling;

    }

    return lastNode;
}

void removeNode(NODE *toRemove){
    if(strcmp(toRemove->name, "/") != 0) {
        if(toRemove != cwd) {
            if(isEmpty(toRemove)) {
                NODE *olderSibling = getOlderSibling(toRemove);
                if (olderSibling) {
                    olderSibling->sibling = toRemove->sibling;
                    free(toRemove);
                } else {
                    toRemove->parent->child = toRemove->sibling;
                }
            } else {
                printf("Could not remove %s, it is non empty\n", toRemove->name);
            }
        } else {
            printf("Cannot remove %s, it is the current working directory\n", toRemove);
        }
    } else {
        printf("Unable to remove root\n");
    }
}

void rm(){
    NODE *toRemove = getNode(pathname, cwd);
    if(toRemove){
        if(toRemove->type == 'F'){
            removeNode(toRemove);
        } else {
            printf("Could not remove %s, it is a Directory\n", toRemove->name);
        }
    }
}

void cd(){
    if(pathname[0] == '\0'){
        cwd = root;
    } else {
        NODE *newNode = getNode(pathname, cwd);
        if (newNode) {
            cwd = newNode;
        }
    }
}

void printNode(NODE *node){
    printf("%c %s\n", node->type, node->name);
}

void ls(){
    if(pathname[0] == '\0'){
        NODE *cur = cwd->child;
        while(cur){
            printNode(cur);
            cur = cur->sibling;
        }
    } else {
        NODE *cur = getNode(pathname, cwd);

        if(cur) {
            cur = cur->child;
            while(cur){
                printNode(cur);
                cur = cur->sibling;
            }
        }

    }
}

void pwdHelper(NODE *directory){
    if(strcmp(directory->name, "/") == 0){
        printf("/");
    } else {
        pwdHelper(directory->parent);
        printf("%s/", directory->name);
    }
}

void pwd(){
    pwdHelper(cwd);
    printf("\n");
}

void savePath(NODE *curNode, FILE *file){
    if(curNode->name == "/"){
        fputc('/', file);
    } else {
        savePath(curNode->parent, file);
        fputs(curNode->name, file);
        fputc('/', file);
    }
}

void saveHelper(NODE *curNode, FILE *file, char path[128]){
    if(curNode) {
        fputc(curNode->type, file);
        fputc(' ', file);
        fputs(path, file);
        fputs(curNode->name, file);
        fputc('\n', file);

        char curPath[128] = "";
        strcpy(curPath, path);
        strcat(curPath, curNode->name);

        if(strcmp(curNode->name, "/") != 0){
            strcat(curPath, "/");
        }

        if(curNode->type == 'D') {
            curNode = curNode->child;

            while (curNode) {
                if(curPath[128] == '\0') {
                    saveHelper(curNode, file, curPath);
                    curNode = curNode->sibling;
                } else {
                    savePath(curNode->parent, file);
                }
            }
        }
    }
}

void save(){
    if(pathname[0] == '\0'){
        strcpy(pathname, "directory");
    }

    FILE *file = fopen(pathname, "w");

    if(file != NULL) {
        saveHelper(root, file, "");
        fclose(file);
    } else {
        printf("Could not open file %s\n", pathname);
    }

}

void clearTree(NODE *curNode){
    if(curNode){
        clearTree(curNode->child);
        if(strcmp(curNode->name, "/") != 0) {
            removeNode(curNode);
            clearTree(curNode->sibling);
        }
    }
}

void reload(){
    FILE *file = fopen(pathname, "r");

    if(file != NULL){
        clearTree(root);

        while (1) {
            char type = fgetc(file);
            fgetc(file); //consume the space

            if(type == EOF){
                break;
            }

            fgets(pathname, 64, file);

            if(strcmp(pathname, "/\n") != 0) {

                splitLast();

                //remove the trailing newline character
                basename[strlen(basename) - 1] = '\0';


                NODE *parent = getNode(dirname, "/");
                makeChild(parent, basename, type);
            }
        }
    } else {
        printf("Could not open file %s\n", pathname);
    }

    cwd = root;

}

int (*fptr[])() = {(int (*)())mkdir, rmdir, ls, cd, pwd, creat, rm, save, reload};
char *functions[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "save", "reload", "\0"};

int main(){

    root = (NODE *) malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->type = 'D';
    root->parent = root;
    root->sibling = root;
    root->child = 0;

    cwd = root;

    strcpy(pathname, "\0");


    do{
        command[0] = '\0';
        pathname[0] = '\0';

        printf("Enter a command: ");
        fgets(line, 128, stdin);
        sscanf(line, "%s%s", command, pathname);

        int commandFound = 0;

        for(int i = 0; functions[i] != "\0"; i++){
            if(strcmp(command, functions[i]) == 0){

                fptr[i]();
                commandFound = 1;
                break;
            }
        }

        if(!commandFound && strcmp("quit", command) != 0){
            printf("Could not find command %s\n", command);
        }

    }while(strcmp(command, "quit"));

    save();


    return 0;
}