#include <stdio.h>
#include <stdlib.h>
#include "type.h"

PROC proc[NPROC], *readyQueue, *freeList;

int main()
{
    int i;
    PROC *p;
    readyQueue = 0;

    for (i=0; i < NPROC; i++){
        p = &proc[i];
        p->pid = i;
        p->priority = rand() % 10;
        printf("pid=%d priority=%d\n", p->pid, p->priority);
        enqueue(&readyQueue, p);
        printList("readyQ", readyQueue);
    }
}