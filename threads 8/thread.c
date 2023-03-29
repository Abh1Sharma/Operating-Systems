#include "thread.h"
#include "interrupt.h"
#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
//#include "lab3.h"
// HELPER FUNCTIONS DECLARATION
// LAB2 redone parts s

#define NTHREADS   128
#define FREEIF(x) if (x) \
{ \
    free(x); \
    x = NULL; \
}
// Test statusSet

typedef enum {
    BLOCKED_STATE = 1,
    EXITED_STATE = 2,
    RUNNING_STATE = 3,
    READY_STATE = 4,
    KILL_STATE = 5,
    WAIT_STATE = 6,
    TEST = 7
} thread_state;
// val add is irrel
//typedef int Tid;
//#define THREAD_MAX_THREADS 1024
//#define THREAD_MIN_STACK 32768
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
    //
    thread_state thdstatus;
    // context struct ptr
    ucontext_t threadcontext;
    void *stackpoint;
    //stp points to pos in arm
    struct wait_queue *waitQueue;
    Tid threadid;
    Tid exitid;
    int exitstatusSet;
    int stVal; // flag R
    int call; // called
    int statusT;
    struct thread_state * childptr; // value of next pointter
    struct thread_state * fatherthread; // redundant R
} StThreadInfo;

// implementing queue w ptr to next and thread info strct
struct Queue *readyQueue;

typedef struct Queue {
    struct Queue *nextnode;
    StThreadInfo *threadinfo;
} StQueue;

// HELPER FUNCTIONS DECLARATION
// LAB2 redone parts 
// new funcs, get curr Id, add the queue w data into it, delete from the queye, and free remaining 
Tid GetFreeID();
Tid PopElementQ(StQueue* waitQueue);
void PushToQ(StQueue *waitQueue, StThreadInfo *threadinfo);
void PopOutQ(StQueue* waitQueue, Tid threadid);
void ClearQ(Tid threadid);
Tid FreeExitedThd();
Tid YieldThd(int statusSet);
// threadinfos --> change in approach from l2 -> not using several threads, less queing issues
StThreadInfo *threadinfos[THREAD_MAX_THREADS] = {
    NULL // set to null
    }; 

// ptr strcut funct to create queue, lab3use
struct wait_queue {
    // sep file imple
    // /* ... Fill this in Lab 3 ... */
    StQueue *curwaitqueue;
    StQueue *futureThd;
};

// // DECLARING STRUCT GLOBAL PTRS
// StQueueNode *rearExitQueue;
// StQueueNode *exitQueue;
// StQueueNode *currentQueue;
// StQueueNode *readyQueue;
// Tid thd_num;
// StQueueNode *rearReadyQueue;
// Tid killThdid[THREAD_MAX_THREADS];
// int killthdnum;

void thread_init(void) {
    // set thread state for running, GDB test check 
    StThreadInfo *tmpthd = (StThreadInfo *) malloc(sizeof (StThreadInfo));
    tmpthd->waitQueue = malloc(sizeof (struct wait_queue));
    tmpthd->waitQueue->curwaitqueue = (StQueue*) malloc(sizeof (StQueue));
    tmpthd->threadid = 0;

    tmpthd->thdstatus = RUNNING_STATE;

    getcontext(&(tmpthd->threadcontext));
    threadinfos[0] = tmpthd;

    readyQueue = (StQueue*) malloc(sizeof (StQueue));
    readyQueue->nextnode = NULL;
}

Tid thread_id() {
    for (int i = 0; i < THREAD_MAX_THREADS; i++) {
        if (NULL == threadinfos[i] || RUNNING_STATE != threadinfos[i]->thdstatus)
            continue;
        return i;
    }
    return THREAD_INVALID;
}


void thread_stub(void (*thread_main)(void *), void *arg) {
    int interrOn = interrupts_set(true);
    thread_main(arg);  
    interrupts_set(interrOn);
    thread_exit(0);
}

Tid thread_create(void (*fn) (void *), void *parg) {
    int interrOn = interrupts_set(false);

    if (THREAD_NOMORE == GetFreeID()) {
        interrupts_set(interrOn);
        return THREAD_NOMORE;
    }
// 
    
       
                  
    // }
    // THREAD_NOMORE = true;
    // if(THREAD_NOMORE == true){
    //     interrupts_set(interrOn);
    //     return THREAD_NOMORE;
    // }


    void *thdstack = (void*) malloc(THREAD_MIN_STACK);
    StThreadInfo* threadinfo = (StThreadInfo*) malloc(sizeof (StThreadInfo)); 

    threadinfo->threadid = GetFreeID();


    // for (int i = 0; i < THREAD_MAX_THREADS; i++) {
    //     if (NULL == threadinfos[i] || (RUNNING_STATE != threadinfos[i]->thdstatus && READY_STATE != threadinfos[i]->thdstatus && BLOCKED_STATE != threadinfos[i]->thdstatus)){
    //         threadinfo->threadid = i;
    //         }                
    // }
    // threadinfo->threadid = ;
    threadinfo->waitQueue = malloc(sizeof (struct wait_queue));
    threadinfo->waitQueue->curwaitqueue = (StQueue*) malloc(sizeof (StQueue));
    threadinfo->thdstatus = READY_STATE;
    threadinfo->stackpoint = thdstack;
    getcontext(&(threadinfo->threadcontext));

    thdstack = thdstack + THREAD_MIN_STACK;
    thdstack = thdstack - 0x8;
    //REG_RIP is The entry function of the context
    //        printf("stackPtr:%d",);
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RIP] = (unsigned long) thread_stub;
    //        printf("stackPtr:%d",);
    //REG_RDI save calling thread function point
     //       printf("stackPtr:%d",);
    //REG_RSI is ave thread param
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RSI] = (unsigned long) parg;
    //sp is Stack pointer
    //        printf("stackPtr:%d",);
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RSP] = (unsigned long) thdstack;
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RDI] = (unsigned long) fn;
   

//  if (killthdnum == 0 || killthdnum == -1 || killthdnum == -2 || killthdnum == -3 || killthdnum == -4) {
//         curnode->threadinfo.threadid = thd_num;
//         thd_num = thd_num+1;
//         //printf("num: %d",thdnum);
//     } 
    threadinfos[threadinfo->threadid] = threadinfo;
    PushToQ(readyQueue, threadinfo);

    interrupts_set(interrOn);

    return threadinfo->threadid;
}

Tid thread_yield(Tid tidThds) {
    int interrOn = interrupts_set(false);
    FreeExitedThd();

    int isvalid = false;
    if ((tidThds >= 0 && tidThds < THREAD_MAX_THREADS)) 
        isvalid = true;
    int self = false;
    if ((THREAD_SELF == tidThds) || (isvalid && tidThds == thread_id()))
        self = true;

   
    int setVal = false;
    if ((isvalid) && (NULL != threadinfos[tidThds]) && (READY_STATE == threadinfos[tidThds]->thdstatus))
        setVal = true;

    Tid readyidinfo = 0;
    Tid runidinfo = thread_id();
    if (self) {
        interrupts_set(interrOn);
        return runidinfo;
    }
    else if (THREAD_ANY == tidThds)
        readyidinfo = PopElementQ(readyQueue);
    else if (setVal)
        readyidinfo = tidThds;
    else {
        interrupts_set(interrOn);
        return THREAD_INVALID;
    }

    if (THREAD_NONE == readyidinfo) {
        interrupts_set(interrOn);
        return THREAD_NONE; 		
    }

    StThreadInfo* readythd = threadinfos[readyidinfo];
    PopOutQ(readyQueue, readyidinfo); 
    FreeExitedThd();

    int isyield = false;
    getcontext(&(threadinfos[runidinfo]->threadcontext));
    if (isyield) {
        isyield = false;
        interrupts_set(interrOn);
        return readythd->threadid;
    } else {
        isyield = true;
        readythd->thdstatus = RUNNING_STATE;
        threadinfos[runidinfo]->thdstatus = READY_STATE;
        PushToQ(readyQueue, threadinfos[runidinfo]);
        setcontext(&(readythd->threadcontext));
    }

    interrupts_set(interrOn);
    return THREAD_NONE;
}

void thread_exit(int statusSet) {
    int interrOn = interrupts_set(false);
    FreeExitedThd();
    interrupts_set(interrOn); 
    ClearQ(thread_id());

    thread_wakeup(threadinfos[thread_id()]->waitQueue, 1);
    threadinfos[thread_id()]->exitid = thread_id();
    threadinfos[thread_id()]->exitstatusSet = statusSet;
    threadinfos[thread_id()]->thdstatus = EXITED_STATE;
    if (THREAD_NONE == YieldThd(statusSet)) {
        exit(0);
    }

    interrupts_set(interrOn);
}

Tid thread_kill(Tid tid) {
    int interrOn = interrupts_set(false);
    int valid_id = 0;
    if ( (NULL != threadinfos[tid]) && (tid != thread_id())){
        if(tid >= 0 && tid < THREAD_MAX_THREADS){
            valid_id = 1;
        }
    }
        
    if (valid_id==0) {
        interrupts_set(interrOn);
        return THREAD_INVALID;
    }

    threadinfos[tid]->thdstatus = EXITED_STATE;
    PopOutQ(readyQueue, tid);

    thread_wakeup(threadinfos[tid]->waitQueue, 1);
    ClearQ(tid);

    FreeExitedThd();
    interrupts_set(interrOn);
    return tid;
}

/// see lab3.c file for separate compilation... cleared work for lab2. 
// ece344-tester 3,2
/*******************************************************************************************************************************************************************************************************
 * Important: The rest of the statusSet should be implemented in Lab 3. *
 ***************************************************************************************************************************************************************************************************/

struct wait_queue * wait_queue_create() {
    int interrOn = interrupts_set(false);
    struct wait_queue *waitqueue = malloc(sizeof (struct wait_queue));
    waitqueue->curwaitqueue = (StQueue*) malloc(sizeof (StQueue));
    waitqueue->curwaitqueue->nextnode = NULL;

    interrupts_set(interrOn);
    return waitqueue;
}

void wait_queue_destroy(struct wait_queue *queue) {
    if((PopElementQ(queue->curwaitqueue) != -1 && PopElementQ(queue->curwaitqueue) != THREAD_NONE )){
    do{
        Tid id = PopElementQ(queue->curwaitqueue);
        PopOutQ(queue->curwaitqueue, id);
    } while (THREAD_NONE != PopElementQ(queue->curwaitqueue));
    }
    FREEIF(queue->curwaitqueue);
    FREEIF(queue);
}

Tid thread_sleep(struct wait_queue *waitQueue) {
    if (NULL == waitQueue)
        return THREAD_INVALID;


    int issleep = 0;
    int interrOn = interrupts_set(false);

    Tid readyidinfo = PopElementQ(readyQueue);

    if (readyidinfo == THREAD_NONE) {
        interrupts_set(interrOn);
        return THREAD_NONE;
    }

    StThreadInfo* readythdinfo = threadinfos[readyidinfo];
    StThreadInfo* waitthdinfo = threadinfos[thread_id()];
    getcontext(&(waitthdinfo->threadcontext));

    if (issleep == 1) {
        issleep = 0;
        FreeExitedThd();
        interrupts_set(interrOn);
        return readyidinfo;
    } else {
        issleep = 1;
        PushToQ(waitQueue->curwaitqueue, waitthdinfo); 
        PopOutQ(readyQueue, readythdinfo->threadid); 
        readythdinfo->thdstatus = RUNNING_STATE;
        waitthdinfo->thdstatus = BLOCKED_STATE;
        setcontext(&(readythdinfo->threadcontext));
    }

    interrupts_set(interrOn);
    return readyidinfo;
}

int thread_wakeup(struct wait_queue *waitQueue, int isVal) 
{
    if(NULL == waitQueue  || NULL == waitQueue->curwaitqueue || NULL == waitQueue->curwaitqueue->nextnode) 
        return 0;

    int interrOn = interrupts_set(false);
    int incr = 0;

    if(isVal == 0) 
    {
        Tid curid;
        if(THREAD_NONE != (curid = PopElementQ(waitQueue->curwaitqueue)))
        {
            threadinfos[curid]->thdstatus = READY_STATE;
            PopOutQ(waitQueue->curwaitqueue, curid); 
            PushToQ(readyQueue, threadinfos[curid]);
        }

        interrupts_set(interrOn);
        return 1;
    }
    else 
    {
        Tid curid;
        //// change before checkout
        do
        {
            incr++;
            threadinfos[curid]->thdstatus = READY_STATE;
            PopOutQ(waitQueue->curwaitqueue, curid); 
            PushToQ(readyQueue, threadinfos[curid]);            
        }while (THREAD_NONE != (curid = PopElementQ(waitQueue->curwaitqueue)));

        interrupts_set(interrOn);
        return incr;
    }

    interrupts_set(interrOn);
	return 0;
}
////////////////////////////////////////***************/////////////////////
// Thread_wait takes in tid and the exit_status set bit
Tid thread_wait(Tid tid, int *exit_statusSet) {
// if tid greater than 0, and null is not equal to thread infos at tid
    if (!(tid >= 0 && tid < THREAD_MAX_THREADS) && (NULL != threadinfos[tid]) && (tid != thread_id())) {
        return THREAD_INVALID;
    }

    int interrOn = interrupts_set(false);
    
    thread_sleep(threadinfos[tid]->waitQueue);
    if (exit_statusSet != NULL) {
          // if exit_status set does not equal null return null

    
    }


    interrupts_set(interrOn);
    return tid;
}

struct lock {
      // structure wait_queue in tid thread
    struct wait_queue* waitQueue;
    Tid thread;
    int id;
};

struct lock* lock_create() {
      // structure lock_create, int interrupts_set 

    int interrOn = interrupts_set(false);
    struct lock *lockinfo;

    lockinfo = malloc(sizeof (struct lock));
    lockinfo->waitQueue = wait_queue_create();
    lockinfo->id = 0;

    interrupts_set(interrOn);
    return lockinfo;
}
 // interrOn equals interrupts_set, leads to void lock_destroy for structure lock

void lock_destroy(struct lock *lockinfo) {
    int interrOn = interrupts_set(false);
      // if lock info is id, wait_queue_destory, FREEIF lockinfo
    if (0 == lockinfo->id) {
        wait_queue_destroy(lockinfo->waitQueue);
        FREEIF(lockinfo);
    }

    interrupts_set(interrOn);
}
	// interOn equals interrupts_set if void lock_acquire 
void lock_acquire(struct lock *lockinfo) {
    int interrOn = interrupts_set(false);

    while (1 == lockinfo->id) {
        thread_sleep(lockinfo->waitQueue);
    }

    lockinfo->id = 1;
    lockinfo->thread = thread_id();

    interrupts_set(interrOn);
}
// 	// interrOn equals interrupts_sets, void lock_releases 
void lock_release(struct lock *lockinfo) {
    int interrOn = interrupts_set(false);
	// if lockinfo is id and thread_id, then lockinfo is thread and lockinfo is id
    if (1 == lockinfo->id && thread_id() == lockinfo->thread) {
        lockinfo->id = 0;
        thread_wakeup(lockinfo->waitQueue, 1);
    }

    interrupts_set(interrOn);
}
 // structure cv starts wait_queue when interrupts_set 
struct cv {
    struct wait_queue *waitQueue;
};
// structure cv_create, when interrOn equals interrupts_set, the cvinfo equal malloc, cvinfo is waitQueue equals wait_queue_create 
struct cv *cv_create() {
    int interrOn = interrupts_set(false);
    struct cv *cvinfo = malloc(sizeof (struct cv));
    if(interrOn){
        if(cvinfo->waitQueue == NULL){
            cvinfo->waitQueue = wait_queue_create();
        }
    }
    interrupts_set(interrOn);
    return cvinfo;
}
 // void cv_destroy if interrOn equals interrupts_set
void cv_destroy(struct cv *cvinfo) {
    int interrOn = interrupts_set(false);

    if (NULL == cvinfo->waitQueue->curwaitqueue->nextnode) {
 // if null equals cvfo is waitQueue is curwaitqueue is nextnode, then wait_queue_destory is FREEIF

        wait_queue_destroy(cvinfo->waitQueue);
        FREEIF(cvinfo);
    }

    interrupts_set(interrOn);
}
 // void cv_wait if interrOn equals interrupts_set
void cv_wait(struct cv *cvinfo, struct lock *lockinfo) {
    int interrOn = interrupts_set(false);
      // if thread_id equals lockinfo is thread, then lock_release and thread_sleep
    if (thread_id() == lockinfo->thread) {
        lock_release(lockinfo);
        thread_sleep(cvinfo->waitQueue);
    }

    lock_acquire(lockinfo);
    interrupts_set(interrOn);
}
 // void cv_signla if interrOn equals interrupts_set, if thread_id equals lockinfo is thread, then thread_wakeup
void cv_signal(struct cv *cvinfo, struct lock *lockinfo) {
    int interrOn = interrupts_set(false);
    if (thread_id() == lockinfo->thread)
        thread_wakeup(cvinfo->waitQueue, false);

    interrupts_set(interrOn);
}
 // void cv_broadcast when interrOn equals interrupts_set, if lockinfo is thread, equals thread_id, then thread_wakeup

void cv_broadcast(struct cv *cvinfo, struct lock *lockinfo) {

    int interrOn = interrupts_set(false);
    if (lockinfo->thread == thread_id())
        thread_wakeup(cvinfo->waitQueue, true);


    interrupts_set(interrOn);
}



/////////////////HELPER FUNCTIONS///////////////////////////////////

Tid GetFreeID() {
    for (int i = 0; i < THREAD_MAX_THREADS; i++) {
        if(NULL== threadinfos[i]){
            return i;
        }
        if(RUNNING_STATE != threadinfos[i]->thdstatus){ 
            if (READY_STATE != threadinfos[i]->thdstatus && BLOCKED_STATE != threadinfos[i]->thdstatus){
                return i;
            }
        }
        
    }
    return THREAD_NOMORE;
}
 // Tid if null is not equal to waitQueue and null is not equal to waitQueue then nextnode

Tid PopElementQ(StQueue* waitQueue) {
    if (NULL != waitQueue && NULL != waitQueue->nextnode) {
        int id = waitQueue->nextnode->threadinfo->threadid;
        return id;
    }
    else
        return THREAD_NONE;
}


//void PushToQ if null is equal to waitQueue
void PushToQ(StQueue *waitQueue, StThreadInfo *threadinfo) {
    if (NULL == waitQueue)
        return;

    StQueue *newthdinfo = (StQueue *) malloc(sizeof (StQueue));
    newthdinfo->threadinfo = threadinfo;
    newthdinfo->nextnode = NULL;
    if (NULL == waitQueue->nextnode)
        waitQueue->nextnode = newthdinfo;
    else {
        StQueue *tempinfo = waitQueue;
        // cyclical switching if null
        do{tempinfo = tempinfo->nextnode;} while(NULL != tempinfo->nextnode);
        tempinfo->nextnode = newthdinfo;
    }
}
 // void PopOutQ if null is equal to waitQueue or null is equal to waitQueue then nextnode 
void PopOutQ(StQueue* waitQueue, Tid threadid) {
    if (NULL == waitQueue || NULL == waitQueue->nextnode)
        return;

    StQueue *previnfo = waitQueue;
    StQueue *currinfo = previnfo->nextnode; 
  // while threadid is not equal to currinfo is threadinfo is threadid and null is not equal to currinfo is nextnode
    if(threadid != currinfo->threadinfo->threadid && NULL != currinfo->nextnode){
    do {
        previnfo = currinfo;
        currinfo = currinfo->nextnode;
    } while (threadid != currinfo->threadinfo->threadid && NULL != currinfo->nextnode);
    }

     // if threadid is eaual to currinfo then threadinfo is threadid then previous info is nextnode equal is equal to currinfo is nextnode then FREEIF
    // node cycle switching
    if (threadid == currinfo->threadinfo->threadid) {
        previnfo->nextnode = currinfo->nextnode;
        FREEIF(currinfo);
    }
}
 // void ClearQ for i is greater than thread_max_threads
void ClearQ(Tid threadid) {
    for (int i = 0; i < THREAD_MAX_THREADS; i++) {
          // if null is not equal to threadinfos then StQueue is equals to threadinfos

          // if null is not equal to currinot then nextnode
        if (NULL != threadinfos[i]) {
            StQueue *currinfo = threadinfos[i]->waitQueue->curwaitqueue;
            if (NULL != currinfo->nextnode) {
                currinfo = currinfo->nextnode;
            }
            // while currinfo is not equal to null

  // if null is not equal to currinfo is then threadinfo and threadid equals currinfo is threadinfo is threadid

            do {
                if (NULL != currinfo->threadinfo && threadid == currinfo->threadinfo->threadid) {
                    PopOutQ(threadinfos[i]->waitQueue->curwaitqueue, threadid);
                }

                currinfo = currinfo->nextnode;
            }while (currinfo != NULL);
        }
    }
}


 //Tid FreeExitedTdh for i is lesser than thread_max_thread 
Tid FreeExitedThd() {
    for (Tid tidInd = 0; tidInd < THREAD_MAX_THREADS; tidInd++) {
         // if null is not equal to threadinfos and EXITED_STATE equals threadinfos is thdstatus
        if (NULL != threadinfos[tidInd] && EXITED_STATE == threadinfos[tidInd]->thdstatus) {
              // readyQueue is threadinfo in stackpoint, then wait_queue_destory to waitQueue, then FREEIF
            PopOutQ(readyQueue, tidInd);
            FREEIF(threadinfos[tidInd]->stackpoint);
            wait_queue_destroy(threadinfos[tidInd]->waitQueue);
            FREEIF(threadinfos[tidInd]);

            return tidInd;
        }

    }

    return THREAD_NONE;
}


 // Tid ready_id is equal to readyQueue is THREAD_NONE equals ready_id, then return ready_id
Tid YieldThd(int statusSet) {
    Tid ready_id = PopElementQ(readyQueue); 
    if (THREAD_NONE == ready_id) {
        int readybit = ready_id;
        while(readybit == ready_id){
            // poll on the scene till we can yield lol
            return ready_id;
            break; 
        }
        
    }

    threadinfos[ready_id]->thdstatus = RUNNING_STATE;
    threadinfos[ready_id]->exitstatusSet = statusSet;
    PopOutQ(readyQueue, threadinfos[ready_id]->threadid); 
    setcontext(&(threadinfos[ready_id]->threadcontext));

    return ready_id;
}

