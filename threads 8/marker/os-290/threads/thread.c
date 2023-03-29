#include "thread.h"
#include "interrupt.h"
#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
//#include "lab3.h"
#define NTHREADS   128
#define FREEIF(x) if (x) \
{ \
    free(x); \
    x = NULL; \
}
// this variable Free define is from geeksforgeeks, used for freeing and in lab23

typedef enum {
    BLOCKED_STATE = 0,
    EXITED_STATE = 1,
    RUNNING_STATE = 2,
    READY_STATE = 3,
    WAITING_STATE = 4,
    KILL_STATE = 5
} thread_state;
// val add is irrel
/* This is the thread control block */
//typedef int Tid;
//#define THREAD_MAX_THREADS 1024
//#define THREAD_MIN_STACK 32768
/*This is the thread control block */
// TCB has the following fields: Geeks4Geeks
/*  Thread ID -> unique identifier assigned by OS to thread when being created
    Thread State -state of thread, can vary
    CPU information -> how far thread progress and data being used
    Thread priority  -> weight of thread over other threads -> helps select READY from queue
    Pointer to creation process - parent, Pointer to chile */
//struct ucontext *uc_link;
//    sigset_t         uc_sigmask;
//    stack_t          uc_stack;
//    mcontext_t       uc_mcontext;
//extern int setcontext (const ucontext_t *__ucp) __THROWNL;

typedef struct {
    /* ... Fill this in ... */
    void *stackpoint;
    //stp points to pos in arm
    ucontext_t threadcontext;
    // context struct ptr
    thread_state thd_state;
    Tid threadid;
    int stVal; // flag R
    int call; // called
    int statusT;
    struct thread_state * childptr; // value of next pointter
    struct thread_state * fatherthread; // redundant R

} StThreadInfo;

// implementing queue w ptr to next and thread info strct

typedef struct QueueNode {
    struct QueueNode * nextnode;
    StThreadInfo threadinfo;
} StQueueNode;

/* This is the wait queue structure */
struct wait_queue {
    // sep file imple
    // /* ... Fill this in Lab 3 ... */

};


// ptr strcut funct to create queue, lab3use
struct wait_queue *wait_queue_create();

struct lock {
    /* ... Fill this in ... */

};
// DECLARING STRUCT GLOBAL PTRS
StQueueNode *rearExitQueue;
StQueueNode *exitQueue;
StQueueNode *currentQueue;
StQueueNode *readyQueue;
Tid thd_num;
StQueueNode *rearReadyQueue;
Tid killThdid[THREAD_MAX_THREADS];
int killthdnum;

StQueueNode *GetMsgFromQueue(StQueueNode **curnode, StQueueNode **rearnode, Tid threadid) {

    StQueueNode * prevNode = NULL;
    StQueueNode * tempNode = *curnode; // temp node points to current node position, cycle through looking for null
    if (tempNode != NULL) {
        do {
            // if current Temp node isnt equal to curr threadID, move on
            if (!(tempNode->threadinfo.threadid == threadid)) // if (tempNode->threadinfo.threadid == threadid)
            {
                prevNode = tempNode;
                //            printf('prev address: %d, temp add: %d \n', &prevNode, &tempNode);
                tempNode = tempNode->nextnode;
            }
            // if the prev node was null and endnode is reached, switche end node w next
            if (prevNode == NULL) {
                if (tempNode == *rearnode) {
                    *rearnode = tempNode->nextnode;
                }
                // else move curr to next
                *curnode = tempNode->nextnode;
            } 
            else    // all other cases we keep moving and replace w prev if we matched
            {
                prevNode->nextnode = tempNode->nextnode;
                if (tempNode == *rearnode) {
                    *rearnode = prevNode;
                }

            }

            return tempNode;


        } while (tempNode != NULL);
    }

    return NULL;
}

void thread_init(void) {
    /* your optional code here */
    // setting all globalptrs to null and malloc size
    thd_num = 1;
    currentQueue = malloc(sizeof (StQueueNode));
    currentQueue->threadinfo.threadid = 0;
    // set thread state for running, GDB test check 
    currentQueue->threadinfo.thd_state = RUNNING_STATE;
    currentQueue->threadinfo.stackpoint = NULL;
    currentQueue->nextnode = NULL;
    readyQueue = NULL;
    rearExitQueue = NULL;
    rearReadyQueue = NULL;
    exitQueue = NULL;
    killthdnum = 0;
    // set to 0 for now, alter by threadsize when exit
    //

}
/* Return the thread identifier of the currently running thread */
Tid thread_id() {
    // get currThreadID from curr queue
    return currentQueue->threadinfo.threadid;   
}

/* New thread starts by calling thread_stub. The arguments to thread_stub are
 * the thread_main() function, and one argument to the thread_main() function. 
 */
void thread_stub(void (*thread_main)(void *), void *arg) {
    //    printf("argP: %p", arg);
    thread_main(arg); // call thread_main() function with arg
    thread_exit(0);

}
/// to implement que we need to check for empty nulls, do the temp switch replace and check others
/// TODO clear clutter COMMENTS
// pushing current msg into the queue, if the curnode isnt nll, do replace iterate, else switch set to rear

Tid PushMsgToQueue(StQueueNode **curnode, StQueueNode **rearnode, StQueueNode *tmpnode) {
    // if ptr to ptr to current node isnt null it means msg que is full rn, need to get 2nd ptr to rear's next to get tmp to set og rear
    if (*curnode != NULL) {
        (*rearnode)->nextnode = tmpnode;
        *rearnode = tmpnode;
    } 
    else  //curr ptr2ptr IS null --> replace w read w temp pos placeholder
    {
        *curnode = tmpnode;
        //printf()
        *rearnode = tmpnode;
        //
    }

    return tmpnode->threadinfo.threadid;
}
/// TODO clear clutter COMMENTS

/* thread_create should create a thread that starts running the function
 * fn(arg).
 *
 * Upon success, return the thread identifier.
 * Upon failure, return the following:
 *
 * THREAD_NOMORE: no more threads can be created.
 * THREAD_NOMEMORY: no more memory available to create a thread stack. */
Tid thread_create(void (*fn) (void *), void *parg) {
    //	setting, clearing all var and stack state
    if (thd_num > THREAD_MAX_THREADS-1) {
        return THREAD_NOMORE;
    }
    if(thd_num == 0){
        return THREAD_FAILED;
    }
    // set context, GDB test vals
    ucontext_t curContext;
    curContext.uc_stack.ss_sp = malloc(THREAD_MIN_STACK);
    
    // allocate node struct
    StQueueNode * curnode = (StQueueNode *) malloc(sizeof (StQueueNode));
    
    if (curnode == NULL) {
        return THREAD_NOMEMORY;
    }
    // set stackptr size
    
    getcontext(&curContext);
    if (curContext.uc_stack.ss_sp == NULL) {
        FREEIF(curnode);
        // free curr node
        return THREAD_NOMEMORY;
    }
    //        setting, clearing all var and stack state
    curContext.uc_stack.ss_size = THREAD_MIN_STACK;
    curContext.uc_stack.ss_flags = 0;
        // push stack ptr to end of stack from where it is relativelt located
    unsigned long int * stackpt = (unsigned long int *) (curContext.uc_stack.ss_sp + curContext.uc_stack.ss_size);
    //        printf("stackPtr:%d",);
    //        setting stack ptr to be at address of -8bytes in stack, replace and push later
    //stackpt = (unsigned long int *) (((unsigned long int) &stackpt -0x32) - 0x8);
    //                                                              .. set first 4 bits to 0, 0x8 is the offset
    stackpt = (unsigned long int *) (((unsigned long int) stackpt - 16 ) - 0x8);
    //REG_RIP is The entry function of the context
    //        printf("stackPtr:%d",);
    curContext.uc_mcontext.gregs[REG_RIP] = (unsigned long int) thread_stub;
    //        printf("stackPtr:%d",);
    //REG_RDI save calling thread function point
    curContext.uc_mcontext.gregs[REG_RDI] = (unsigned long int) fn;
    //       printf("stackPtr:%d",);
    //REG_RSI is ave thread param
    curContext.uc_mcontext.gregs[REG_RSI] = (unsigned long int) parg;
    //sp is Stack pointer
    //        printf("stackPtr:%d",);
    curContext.uc_mcontext.gregs[REG_RSP] = (unsigned long int) stackpt;
    //        printf("stackPtr:%d",);

    if (killthdnum == 0 || killthdnum == -1 || killthdnum == -2 || killthdnum == -3 || killthdnum == -4) {
        curnode->threadinfo.threadid = thd_num;
        thd_num = thd_num+1;
        //printf("num: %d",thdnum);
    } 
    else 
    {
        int prevThread = killthdnum - 1;
        curnode->threadinfo.threadid = killThdid[prevThread];
        //                printf("curr: ");
        killthdnum= killthdnum - 1;
        //                printf("kill num: %d",killthdnum);
        thd_num = thd_num + 1;
    }
    curnode->nextnode = NULL;
    curnode->threadinfo.thd_state = READY_STATE;
    PushMsgToQueue(&readyQueue, &rearReadyQueue, curnode);
    curnode->threadinfo.threadcontext = curContext;
    curnode->threadinfo.stackpoint = curContext.uc_stack.ss_sp;
    // set next node to null cause gonna call create again next time

    return curnode->threadinfo.threadid;
}
/// TODO clear clutter COMMENTS

void exitthdfromqueue() {
    // save que point and make a temp to set next
    StQueueNode *tmpnode = exitQueue;
    while (tmpnode != NULL) {
        if (tmpnode->threadinfo.threadid == 0) 
        {
            tmpnode = tmpnode->nextnode;
        }
      
        else if(tmpnode != NULL) 
        {
        StQueueNode * tmpcode;
        tmpcode = GetMsgFromQueue(&tmpnode, &rearExitQueue, tmpnode->threadinfo.threadid);
        tmpcode->nextnode = NULL;
        // get threadid in kill array and free
        killThdid[killthdnum] = tmpcode->threadinfo.threadid;

        FREEIF(tmpcode->threadinfo.stackpoint);
        FREEIF(tmpcode);
        killthdnum = killthdnum+1;
        thd_num = thd_num - 1;
        }
    }

    exitQueue = NULL;
}

Tid thread_yield(Tid yieldid) {

    exitthdfromqueue();
    bool isyielded = false;
    if (yieldid == THREAD_SELF || yieldid == thread_id()) 
    {
        //        return thread_id(1);
        return thread_id();
    }

    if (yieldid == THREAD_ANY && readyQueue == NULL) 
    {
        return THREAD_NONE;
    } else if (yieldid == THREAD_ANY) 
    {
        yieldid = readyQueue->threadinfo.threadid;
    }

    StQueueNode * node = GetMsgFromQueue(&readyQueue, &rearReadyQueue, yieldid);

    if (node == NULL) 
    {
        return THREAD_INVALID;
    }

    node->nextnode = NULL;
    
    ucontext_t yieldcontext;
    
    getcontext(&yieldcontext);
    
    if (isyielded ==true) {
        return yieldid;
    }

    //        set isyield to setpoint
    isyielded = true;
    currentQueue->threadinfo.threadcontext = yieldcontext;
    currentQueue->threadinfo.thd_state = READY_STATE;

    node->threadinfo.thd_state = RUNNING_STATE;
    PushMsgToQueue(&readyQueue, &rearReadyQueue, currentQueue);

    currentQueue = node;
    setcontext(&(currentQueue->threadinfo.threadcontext));
    
    return yieldid;
}
/// TODO clear clutter COMMENTS
/*
This function ensures that the current thread does not run after this call, i.e., this function should never return. If there are other threads in the system, one of them should be run. If there are no other threads (this is the last thread invoking thread_exit), then the program should exit. In the future, a new thread should be able to reuse this thread's identifier, but only after this thread has been destroyed. The function has no return values. The exit_code can be any integer value provided by the exiting thread.

In this lab, the exit code will not be used. In the next lab, you will arrange for the exit_code to be passed to another thread that waits for this thread to exit.*/
void thread_exit(int exit_code) {
    exitthdfromqueue();
    if (NULL == readyQueue)
    {
        exit(0);
    }

    ucontext_t runningcontext;
    getcontext(&runningcontext);

    currentQueue->threadinfo.threadcontext = runningcontext;
    //        printf("running context state:",);
    currentQueue->threadinfo.thd_state = EXITED_STATE;
    //        printf("running context state:",);
    int ret = interrupts_set(0);
    // set interrupt 0 
    PushMsgToQueue(&exitQueue, &rearExitQueue, currentQueue);
    //        printf("running context state:",);
    interrupts_set(ret);
    //        printf("running context state:",);
    StQueueNode * curcode = GetMsgFromQueue(&readyQueue, &rearReadyQueue, readyQueue->threadinfo.threadid);
    //        printf("running context state:",);
    curcode->threadinfo.thd_state = RUNNING_STATE;
    //        printf("running context state:",);
    curcode->nextnode = NULL;
    //        printf("running context state:",);
    currentQueue = curcode;
    //        printf("running context state:",);
    setcontext(&(currentQueue->threadinfo.threadcontext));

}
/*This function kills another thread whose identifier is tid. The tid can be the identifier of any available thread. The killed thread should immediately exit with an exit code of 9 (this value pays homage to kill -9) when it runs the next time, while the calling thread should continue to execute. Upon success, this function returns the identifier of the thread that was killed. Upon failure, it returns the following:
THREAD_INVALID: alerts the caller that the identifier tid does not correspond to a valid thread, or is the current thread.*/
Tid thread_kill(Tid threadid) {
    exitthdfromqueue();
    StQueueNode * tmpnode = GetMsgFromQueue(&readyQueue, &rearReadyQueue, threadid);
    if (NULL == tmpnode) 
    {
        return THREAD_INVALID;
    }

    // killThdid[killthdnum] = tmpnode->threadinfo.threadid;

    tmpnode->threadinfo.thd_state = EXITED_STATE;
    killThdid[killthdnum] = tmpnode->threadinfo.threadid;
    FREEIF(tmpnode->threadinfo.stackpoint);
    FREEIF(tmpnode);
    // free stptr and currnode
    killthdnum = killthdnum + 1;
    //    printf("killthdnum: %d",killthdnum);
    thd_num = thd_num - 1;
    return threadid;
}
/* ****** CAUTIONARY NOTES: */
/*Be careful. It is dangerous to use memory once it has been freed. 
 * In particular, you should not free the stack of the currently running thread in thread_exit while it is still running. So how will you make sure that the thread stack is eventually deallocated? How will you make sure that another thread that is created in between does not start using this stack (and then you inadvertently deallocate it)? You should convince yourself that your program would work even if you used a debugging malloc library that overwrites a block with dummy data when that block is free()'d.
Be careful. 
 * If you destroy a thread that is holding or waiting on a resource such as a lock (we will be implementing locks in the next lab), problems can occur. For example, deadlock may occur because the thread holding the lock may not have a chance to release the lock. Similarly, if the thread waiting on a lock is destroyed, then the thread releasing the lock may wake up some other thread incorrectly (e.g., due to reusing thread id). For this reason, it is important to ensure that when thread_kill is invoked on a thread, the target thread should not exit immediately. Instead, the target thread should exit when it runs the next time. How will you implement this functionality? In practice, operating systems provide a signal handler mechanism that allows threads to clean up their resources (e.g., locks) before they exit.
What are the similarities and differences between thread_yield and thread_exit? 
 * Think carefully. It will be useful to encapsulate all that is similar in a common function, 
 * which will help reduce bugs, and possibly make your code simpler.*/

/// see lab3.c file for separate compilation... cleared work for lab2. 
// ece344-tester 3,2
/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *wait_queue_create() {
    return NULL;
}

void
wait_queue_destroy(struct wait_queue *wq) {
    // TBD();
    FREEIF(wq);
}

Tid
thread_sleep(struct wait_queue *queue) {
    // TBD();
    // return THREAD_FAILED;
    return 0;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all) {

    return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid, int *exit_code) {
    //TBD();
    return 0;

}

struct lock *
lock_create() {
    /* ... Fill this in ... */
    struct lock *lock;
    lock = malloc(sizeof (struct lock));
    assert(lock);
    //TBD();
    return lock;
}

void
lock_destroy(struct lock *lock) {
    assert(lock != NULL);
    //TBD();

    FREEIF(lock);
}

void
lock_acquire(struct lock *lock) {

    //TBD();
}

void
lock_release(struct lock *lock) {

    // TBD();
}

struct cv {
    /* ... Fill this in ... */

};

struct cv *
cv_create() {
    // struct cv *cv;


    // TBD();

    // return cv;
    struct cv *cv;

    cv = malloc(sizeof (struct cv));
    assert(cv);



    return cv;
}

void
cv_destroy(struct cv *cv) {


    assert(cv != NULL);
    // assert(cv->wq->front == NULL);
    //TBD();


}

void
cv_wait(struct cv *cv, struct lock *lock) {
    // assert(cv != NULL);
    // assert(lock != NULL);

    // TBD();


}

void
cv_signal(struct cv *cv, struct lock *lock) {
    // assert(cv != NULL);
    // assert(lock != NULL);

    // TBD();

}

void
cv_broadcast(struct cv *cv, struct lock *lock) {
    // assert(cv != NULL);
    // assert(lock != NULL);

    // TBD();

}
