/*********** A Multitasking System ************/
#include <stdio.h>
#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;


int printChildren(PROC *parent){
    printf("'%d", parent->child);
    PROC *cur = parent->child;

    while(cur){
        printf("[%d, %s]-> ", cur->pid, status[cur->status]);
        cur = cur->sibling;
        fflush(stdout);
    }

    printf("NULL\n");
    printf("'%d'", parent->child);
    //printf("FINISHED!!!!");
    return 1;
}

int ksleep(int event)
{
    printf("proc %d sleep on %d: ", running->pid, event);
    running->event = event;
    running->status = SLEEP;
    enqueue(&sleepList, running);
    // printsleepList(); // show sleepList contents
    tswitch();
}

int kwakeup(int event)
{
    PROC *temp = 0;
    PROC *p;
    while(p = dequeue(&sleepList)){
        if (p->status == SLEEP && p->event == event){
            printf("WOKE %s", p->pid);
            p->status = READY;
            enqueue(&readyQueue, p);
            printf("wakeup %d : ", p->pid);
            printList("readyQueue", readyQueue);
        }
        else{
            enqueue(&temp, p);
        }
    }
    sleepList = temp;
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
    printf("fFinding p1");

    PROC *cur = readyQueue;
    while(cur){

        if(cur->pid == 1){
            printf("p1 found in ready queue");
            return cur;
        } else {
            cur = cur->sibling;
        }
    }

    cur = freeList;
    while(cur){
        if(cur->pid == 1){
            printf("p1 found in freelist");
            return cur;
        } else {
            cur = cur->sibling;
        }
    }

    cur = sleepList;
    while(cur){
        if(cur->pid == 1){
            printf("p1 Found in sleeplist");
            return cur;
        } else {
            cur = cur->sibling;
        }
    }

    printf("P1 was not found at all");

    return 0;
}

int kexit()
{
    //printf("proc %d in kexit()\n", running->pid);
//    printf("YES");

    if (running->pid == 1){
        printf("P1 never dies\n");
        return -1;
    }

//    printf("P%d for giving children \n", running->pid);
//    printf("\n");
////    printf("P%d children = ", running->pid);
////    printChildren(running);
//    printf("Finished printing");


    /********** DO: give all children to P1 *************
    disposeChild();
    kwakeup(1);   // wakeup P1 if has given any child to P1
    ******************************************************/

//    (1). record pid as exitStatus;
    printf("Setting exit code to pid");
    running->exitCode = running->pid;
    printf("Setting zombie state");
//    (2). become a ZOMBIE;
    running->status = ZOMBIE;

//    (3). send children to P1; wakeup P1 if has sent any child to P1;
    PROC *cur = running->child;

    printf("Waking p1");
    kwakeup(1);
    printf("p1 is woke");

    PROC *p1 = findP1();

    PROC *p1childEnd = p1->child;
    PROC *p1Prev = 0;
    while(p1childEnd){
        p1Prev = p1childEnd;
        p1childEnd = p1childEnd->sibling;
    }

    p1childEnd = p1Prev;
    p1childEnd->sibling = cur;

    while(cur){
        cur->parent = 1;
        cur = cur->sibling;
    }

    printf("P1 = ");
    printChildren(p1);

    kwakeup(running->ppid);   // wakeup parent if it's waiting
    tswitch();
}

int do_exit()
{
    printf("in exit");
    kexit();
}

int kwait(int *status)
{
    PROC *cur = running->child;
    PROC *prev = 0;
    if(cur){
        while(cur){
            if(cur->status == ZOMBIE){
                *status = cur->exitCode;

                if(prev){
                    prev->sibling = cur->sibling;
                } else {
                    running->child = cur->sibling;
                }

                enqueue(freeList, cur);
                return cur->pid;

            } else {
                prev = cur;
                cur = cur->sibling;
            }
        }

        ksleep(running);
    } else {
        return -1;
    }

    // DO THIS: implemet the kwait() function
}

int do_wait()
{
    int pid, status;
    pid = kwait(&status);
    printf("proc %d waited for a ZOMBIE child %d status=%d\n",
           running->pid, pid, status);
}

int body()
{
    int c, CR;
    printf("proc %d resume to body()\n", running->pid);
    while(1){
        printf("***************************************\n");
        printf("proc %d running: Parent=%d  P%d child = ", running->pid, running->ppid, running->pid);

        printChildren(running);

        printf("input a char [f|s|q| w ] : "); // ADD w command
        c = getchar();
        CR=getchar();

        switch(c){
            case 'f': do_kfork();     break;
            case 's': do_switch();    break;
            case 'q': do_exit();      break;
            case 'w': do_wait();      break;
            default :                 break;
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
    p->parent = running;
    p->sibling = 0;
    p->child = 0;

    PROC *cur = running->child;
    PROC *prev = 0;

    while(cur){
        prev = cur;
        cur = cur->sibling;
    }

    if(prev){
        prev->sibling = p;
    } else {
        running->child = p;
    }
    //                    -1   -2  -3  -4  -5  -6  -7  -8   -9
    // kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
    for (i=1; i<10; i++)
        p->kstack[SSIZE - i] = 0;

    p->kstack[SSIZE-1] = (int)body;
    p->saved_sp = &(p->kstack[SSIZE - 9]);




    /******** YOU MUST HAVE DONE THIS in pre-work#1 ********
    enter_child(p);
    *******************************************************/

    enqueue(&readyQueue, p);
    printList("readyQ", readyQueue);
    return p->pid;
}

int init()
{
    //haha
    int i; PROC *p;
    for (i = 0; i < NPROC; i++){
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;
        p->next = p + 1;
    }
    proc[NPROC-1].next = 0;

    freeList = &proc[0];
    readyQueue = 0;
    sleepList = 0;

    // create P0 as the initial running process
    p = running = dequeue(&freeList);
    p->status = READY;
    p->priority = 0;
    p->child = 0;
    p->sibling = 0;
    p->parent = running;
    printList("freeList", freeList);
    printf("init complete: P0 running\n");
}

/*************** main() ***************/
main()
{
    printf("\nWelcome to KCW's 360 Multitasking System\n");
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


