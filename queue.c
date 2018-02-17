#include <stdio.h>
#include "type.h"

int enqueue(PROC **queue, PROC *p)
{
    PROC *prev = 0;
    PROC *cur = *queue;
    while(cur){
        if(cur->priority < p->priority){
            break;
        }
        prev = cur;
        cur = cur->sibling;
    }

    if(prev){
        PROC *sibling = prev->sibling;
        prev->sibling = p;
        p->sibling = sibling;
    } else {
        PROC *sibling = *queue;
        *queue = p;
        p->sibling = sibling;
    }
    // enter p into queue by priority; FIFO if same prioirty
}

PROC *dequeue(PROC **queue)
{
    PROC *first = *queue;

    if(first->sibling){
        *queue = first->sibling;
    }

    *queue = first->parent;

    return first;
    // remove and return first PROC from queue
}

int printList(char *name, PROC *p)
{
    printf("%s = ", name);
    // print list elements as [pid priority] -> ....
    printf("%s = ", name);
    while(p){
        printf("[%d %d]", p->pid, p->priority);
        p = p->sibling;
    }
}