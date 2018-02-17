/*********** A Multitasking System ************/
#include <stdio.h>
#include <stdlib.h>
#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

int kwakeup(int event){
    PROC *cur = sleepList;
    PROC *prev = 0;

    while(cur){
        if(event == cur->event){
            if(prev){
                prev->sibling = cur->sibling;
            } else {
                sleepList = cur->sibling;
            }

            cur->status = READY;

            enqueue(readyQueue, cur);
        }
        prev = cur;
        cur = cur->sibling;
    }
}


int do_switch()
{
    printf("proc %d switching task\n", running->pid);
    tswitch();
    printf("proc %d resuming\n", running->pid);
}

int do_kfork()
{
    int child = kfork();
    if (child < 0)
        printf("kfork failed\n");
    else{
        printf("proc %d kforked a child = %d\n", running->pid, child);
        printList("readyQueue", readyQueue);
    }
    return child;
}

PROC* findP1(){
    PROC *cur = running;
    while(cur){
        if(cur->pid == 1){
            return cur;
        } else {
            cur = cur->sibling;
        }
    }

    return 0;
}

int kexit(){
    int i;
    PROC *p;
    printf("proc %d in kexit()\n", running->pid);
    if (running->pid==1){
        printf("P1 never dies\n");
        return 0;
    }

//    TODO
//    (1). record pid as exitStatus;
    running->exitCode = running->pid;

//    (2). become a ZOMBIE;
    running->status = ZOMBIE;

//    (3). send children to P1; wakeup P1 if has sent any child to P1;
    PROC *cur = running->child;

    kwakeup(1);

    PROC *p1 = findP1();

    if(!p1){
        printf("p1 was not found\n");
        return 0;
    }

    while(cur){
        if(!p1->child) {
            p1->child = cur;
        } else {
            enqueue(&(p1->child), cur);
        }

        cur = cur->sibling;
    }
//    (4). kwakeup(parentPID);  // parent may be sleeping on its PID in wait()
    kwakeup(running->ppid);

    tswitch();

}

char *gasp[NPROC]={
        "Oh! I'm dying ....",
        "Oh! You're killing me ....",
        "Oh! I'm a goner ....",
        "Oh! I'm mortally wounded ....",
};



//int kexit(){
//
//    printf("*************************************\n");
//    printf("proc %d: %s\n", running->pid, gasp[running->pid % 4]);
//    printf("*************************************\n");
//    running->status = FREE;
//    running->priority = 0;
//
//// ASSIGNMENT 3: add YOUR CODE to delete running PROC from parent's child list
//
//    enqueue(&freeList, running);     // enter running into freeList
//    printList("freeList", freeList); // show freeList
//
//    tswitch();
//}

int do_exit()
{
    printf("Exit!!!");
    if (running->pid==1){
        printf("P1 never dies\n");
        return -1;
    }
    kexit();    // journey of no return
}

int body()
{
    int c, CR;
    printf("proc %d starts from body()\n", running->pid);
    int run = 1;
    while(run){
        printf("***************************************\n");
        printf("proc %d running: Parent = %d\n", running->pid, running->ppid);

        // ASSIGNMENT 3: add YOUR CODE to show child list

        printf("input a char [f|s|q] : ");
        c = getchar(); CR=getchar();
        switch(c){
            case 'f': do_kfork();     break;
            case 's': do_switch();    break;
            case 'q': do_exit();      break;
            case 'p': printList("freeList", freeList); printList("running", running); printList("readyQueue", readyQueue); break;
            case 'z': run = 0; break;
        }
    }
}

/*******************************************************
  kfork() creates a child porc; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int kfork()
{
    PROC *p;
    int  i;
    /*** get a proc from freeList for child proc: ***/
    p = dequeue(&freeList);
    if (!p){
        printf("no more proc\n");
        return(-1);
    }

    /* initialize the new proc and its stack */
    p->status = READY;
    p->priority = 1;         // for ALL PROCs except P0
    p->ppid = running->pid;

    //                    -1   -2  -3  -4  -5  -6  -7  -8   -9
    // kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
    for (i=1; i<10; i++)
        p->kstack[SSIZE - i] = 0;

    p->kstack[SSIZE-1] = (int)body;
    p->saved_sp = &(p->kstack[SSIZE - 9]);

/**************** ASSIGNMENT 3  ********************
  add YOUR code to implement the PROC tree as a BINARY tree
  enter_child(running, p);
****************************************************/

    enqueue(&readyQueue, p);
    return p->pid;
}

int init()
{
    int i;
    for (i = 0; i < NPROC; i++){
        proc[i].pid = i;
        proc[i].status = FREE;
        proc[i].priority = 0;
        proc[i].next = (PROC *)&proc[(i+1)];
    }
    proc[NPROC-1].next = 0;

    freeList = &proc[0];
    readyQueue = 0;
    sleepList = 0;

    // create P0 as the initial running process
    running = dequeue(&freeList);
    running->status = READY;
    running->priority = 0;

    running->child = 0;
    running->sibling = 0;
    running->parent = running;

    printf("init complete: P0 running\n");
    printList("freeList", freeList);
}


//int main()
//{
//    int i;
//    PROC *p;
//    readyQueue = 0;
//
//    srand(132145465);
//
//    for (i=0; i < NPROC; i++){
//        p = &proc[i];
//        p->pid = i;
//        p->priority = rand() % 10;
//        printf("pid=%d priority=%d\n", p->pid, p->priority);
//        enqueue(&readyQueue, p);
//        printList("readyQ", readyQueue);
//    }
//}

/*************** main() ***************/
main()
{
    printf("\nWelcome to Hugh's 360 Multitasking System\n");
    init();
    kfork();
    printf("P0: switch task\n");
    tswitch();
    printf("All dead. Happy ending\n");
}

/*********** scheduler *************/
int scheduler()
{
    printf("proc %d in scheduler()\n", running->pid);
    if (running->status == READY)
        enqueue(&readyQueue, running);
    printList("readyQueue", readyQueue);
    running = dequeue(&readyQueue);
    printf("next running = %d\n", running->pid);
}


