#include "thread.h"
#include "interrupt.h"
#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
//#include "lab3.h"
// HELPER FUNCTIONS DECLARATION
// LAB2 redone parts 

#define NTHREADS   128
#define FREEIF(x) if (x) \
{ \
    free(x); \
    x = NULL; \
}

typedef enum 
{
    BLOCKED_STATE=1,
    EXITED_STATE=2,
	RUNNING_STATE=3,
    READY_STATE=4,
    KILL_STATE = 5,
    WAIT_STATE = 6
}thread_state;
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
    thread_state thdstatus;
    // context struct ptr
    ucontext_t threadcontext;
    void *stackpoint;
    //stp points to pos in arm
    struct wait_queue *waitQueue;
    Tid threadid;
    Tid exitid;
    int exitcode;
    int stVal; // flag R
    int call; // called
    int statusT;
    struct thread_state * childptr; // value of next pointter
    struct thread_state * fatherthread; // redundant R
}StThreadInfo;

// implementing queue w ptr to next and thread info strct
struct Queue *readyQueue;

typedef struct Queue 
{
    struct Queue *nextnode;
    StThreadInfo *threadinfo;
}StQueue;

// HELPER FUNCTIONS DECLARATION
// LAB2 redone parts 
Tid GetAvailId ();
Tid GetFrontQueueData(StQueue* waitQueue);
void AddDataToQueue(StQueue *waitQueue, StThreadInfo *threadinfo);
void DelFromQueue(StQueue* waitQueue, Tid threadid);
void DelAllQueueData(Tid threadid);
void FreeThreadData(Tid threadid);
Tid FreeExitedThd();
Tid YieldThd(int code);

StThreadInfo *threadinfos[THREAD_MAX_THREADS] = { NULL }; // all threads 



// function parameters
// new funcs, get curr Id, add the queue w data into it, delete from the queye, and free remaining 
Tid GetAvailId ();
void AddDataToQueue(StQueue* waitQueue, StThreadInfo* threadinfo);
void DelFromQueue(StQueue* waitQueue, Tid threadid);
Tid FreeExitedThd();
Tid YieldThd(int exitcode);
//void DelAllQueueData(Tid threadid);

// ptr strcut funct to create queue, lab3use
/* This is the wait waitQueue structure */
struct wait_queue 
{
    // sep file imple
    // /* ... Fill this in Lab 3 ... */
	StQueue *curwaitqueue; 
};
void DelAllQueueData(Tid threadid);

// struct lock {
//     /* ... Fill this in ... */

// };
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
    StThreadInfo *tmpthd = (StThreadInfo *)malloc(sizeof(StThreadInfo));
    tmpthd->waitQueue = malloc(sizeof(struct wait_queue));
    tmpthd->waitQueue->curwaitqueue = (StQueue*)malloc(sizeof(StQueue));
    tmpthd->threadid = 0;

    tmpthd->thdstatus = RUNNING_STATE;
    
    getcontext(&(tmpthd->threadcontext));
    threadinfos[0] = tmpthd; 

    readyQueue = (StQueue*)malloc(sizeof(StQueue));
    readyQueue->nextnode = NULL; 
    // set to 0 for now, alter by threadsize when exit
    //
}

/* Return the thread identifier of the currently running thread */
Tid thread_id() {	
    // get currThreadID from curr queue
    for(int i = 0; i < THREAD_MAX_THREADS; i++) 
    {
        if(NULL == threadinfos[i]  || RUNNING_STATE != threadinfos[i]->thdstatus)
            continue;
        return i;
    }       
    return THREAD_INVALID; 
}

/* New thread starts by calling thread_stub. The arguments to thread_stub are
 * the thread_main() function, and one argument to the thread_main() function. 
 */
void thread_stub(void (*thread_main)(void *), void *arg) 
{
    int enable = interrupts_set(1);
    //    printf("argP: %p", arg);
    thread_main(arg); // call thread_main() function with arg   
    interrupts_set(enable); 
    thread_exit(0);
}
/// to implement que we need to check for empty nulls, do the temp switch replace and check others
/// TODO clear clutter COMMENTS
// pushing current msg into the queue, if the curnode isnt nll, do replace iterate, else switch set to rear

// Tid PushMsgToQueue(StQueueNode **curnode, StQueueNode **rearnode, StQueueNode *tmpnode) {

/// TODO clear clutter COMMENTS

/* thread_create should create a thread that starts running the function
 * fn(arg).
 *
 * Upon success, return the thread identifier.
 * Upon failure, return the following:
 *
 * THREAD_NOMORE: no more threads can be created.
 * THREAD_NOMEMORY: no more memory available to create a thread stack. */
// changing whole thread creation process
Tid thread_create(void (*fn) (void *), void *parg){
    int enabled = interrupts_set(0);
    if(THREAD_NOMORE == GetAvailId()) 
    {
        interrupts_set(enabled);
        return THREAD_NOMORE;
    }
    
	void *thdstack = (void*)malloc(THREAD_MIN_STACK);
    StThreadInfo* threadinfo = (StThreadInfo*)malloc(sizeof(StThreadInfo)); 			// allocate mem for StThreadInfo control block
    threadinfo->threadid = GetAvailId();
    threadinfo->waitQueue = malloc(sizeof(struct wait_queue));
    threadinfo->waitQueue->curwaitqueue = (StQueue*)malloc(sizeof(StQueue));
    threadinfo->thdstatus = READY_STATE;
    threadinfo->stackpoint = thdstack;
    getcontext(&(threadinfo->threadcontext));	
    
    thdstack = thdstack + (THREAD_MIN_STACK - 8);	
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RIP] = (unsigned long)thread_stub;
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RDI] = (unsigned long)fn;    
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RSI] = (unsigned long)parg; 
    threadinfo->threadcontext.uc_mcontext.gregs[REG_RSP] = (unsigned long)thdstack;

    threadinfos[threadinfo->threadid] = threadinfo;
    AddDataToQueue(readyQueue, threadinfo);
    interrupts_set(enabled);
    return threadinfo->threadid;
}

Tid thread_yield(Tid thdtid) {     
    int enabled = interrupts_set(0);
    FreeExitedThd();
    
    int isvalid = 0;
    if( (thdtid >= 0 && thdtid < THREAD_MAX_THREADS)) // is the thdtid within the valid range (for indexing a specific thread)
        isvalid = 1;
    int self = 0;
    if ((THREAD_SELF == thdtid) || (isvalid && thdtid == thread_id()))
        self = 1;

    // int YIELD_ANY 		= (thdtid == THREAD_ANY);
    int specific = 0;
    if ((isvalid) && (NULL != threadinfos[thdtid]) && (READY_STATE == threadinfos[thdtid]->thdstatus))
        specific = 1; 

    Tid readyidinfo = 0;
    Tid runidinfo = thread_id();
    // figure out which ready threadinfo to yield to 
    if (self) 
    {
        interrupts_set(enabled);
        return runidinfo;
    }		
    else if (THREAD_ANY == thdtid) 			
        readyidinfo = GetFrontQueueData(readyQueue); 												  
    else if(specific) 		
        readyidinfo = thdtid;
    else 
    {
        interrupts_set(enabled);
        return THREAD_INVALID;
    }

    if(THREAD_NONE == readyidinfo) 
    {
        interrupts_set(enabled);
        return THREAD_NONE; // return from GetFrontQueueData(readyQueue)		
    }			 

    StThreadInfo* readythd = threadinfos[readyidinfo];
    DelFromQueue(readyQueue, readyidinfo);			// delete threadinfo from ready waitQueue
    FreeExitedThd(); 

    int isyield = 0;
    getcontext(&(threadinfos[runidinfo]->threadcontext));
    if(isyield) 
    { 			
        isyield = 0;
        interrupts_set(enabled);
        return readythd->threadid;
    }
    else 
    {												
        isyield = 1;
        readythd->thdstatus = RUNNING_STATE;
        threadinfos[runidinfo]->thdstatus = READY_STATE;
        AddDataToQueue(readyQueue, threadinfos[runidinfo]);
        setcontext(&(readythd->threadcontext));	
    }
    
    interrupts_set(enabled);
    return THREAD_NONE;
}
void thread_exit(int code) 
{
    int enabled = interrupts_set(0);
    FreeExitedThd();   
    interrupts_set(enabled);                               // delete any threadinfos waiting to be killed
    DelAllQueueData(thread_id());
    
    thread_wakeup(threadinfos[thread_id()]->waitQueue, 1);
    threadinfos[thread_id()]->exitid = thread_id();
    threadinfos[thread_id()]->exitcode = code;
    threadinfos[thread_id()]->thdstatus = EXITED_STATE;
    // return;
    if(THREAD_NONE == YieldThd(code)) 
    {
        exit(0);         
    }  

    interrupts_set(enabled);
}

/*This function kills another thread whose identifier is tid. The tid can be the identifier of any available thread. The killed thread should immediately exit with an exit code of 9 (this value pays homage to kill -9) when it runs the next time, while the calling thread should continue to execute. Upon success, this function returns the identifier of the thread that was killed. Upon failure, it returns the following:
THREAD_INVALID: alerts the caller that the identifier tid does not correspond to a valid thread, or is the current thread.*/
Tid thread_kill(Tid tid) 
{	
    int enabled = interrupts_set(0);
    int valid_id = 0;
    if ((tid >= 0 && tid < THREAD_MAX_THREADS) && (NULL != threadinfos[tid]) && (tid != thread_id()))
        valid_id = 1;
    if(!valid_id) 
    {
        interrupts_set(enabled);
        return THREAD_INVALID;
    }

    threadinfos[tid]->thdstatus = EXITED_STATE;
    DelFromQueue(readyQueue, tid);

    thread_wakeup(threadinfos[tid]->waitQueue, 1); 
    DelAllQueueData(tid); 

    FreeExitedThd();
    interrupts_set(enabled);
    return tid;
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
/*******************************************************************************************************************************************************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 ***************************************************************************************************************************************************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue * wait_queue_create() {
    int enabled = interrupts_set(0);
	struct wait_queue *waitqueue = malloc(sizeof(struct wait_queue));
    waitqueue->curwaitqueue = (StQueue*)malloc(sizeof(StQueue));
    waitqueue->curwaitqueue->nextnode = NULL; 
    
    interrupts_set(enabled);
	return waitqueue;
}

void wait_queue_destroy(struct wait_queue *queue) {
    while (THREAD_NONE != GetFrontQueueData(queue->curwaitqueue)) 
    {
        Tid id = GetFrontQueueData(queue->curwaitqueue);
        DelFromQueue(queue->curwaitqueue, id);
    }

    FREEIF(queue->curwaitqueue);
	FREEIF(queue);
}

Tid thread_sleep(struct wait_queue *waitQueue) 
{
     if(NULL == waitQueue) 
        return THREAD_INVALID;
    

    int issleep = 0;
    int enabled = interrupts_set(0);
    
    Tid readyidinfo = GetFrontQueueData(readyQueue); 					
    
    if(readyidinfo == THREAD_NONE) 
    {
        interrupts_set(enabled);
        return THREAD_NONE;
    }

    StThreadInfo* readythdinfo = threadinfos[readyidinfo];
    StThreadInfo* waitthdinfo = threadinfos[thread_id()];
    getcontext(&(waitthdinfo->threadcontext));
    
    if(issleep) 
    { 		
        issleep = 0;	
        FreeExitedThd();
        interrupts_set(enabled);
        return readyidinfo;
    }
    else 
    {												
        issleep = 1;
        AddDataToQueue(waitQueue->curwaitqueue, waitthdinfo); // running thread to wait waitQueue
        DelFromQueue(readyQueue, readythdinfo->threadid);	// delete ready thread form ready waitQueue
        readythdinfo->thdstatus = RUNNING_STATE;
        waitthdinfo->thdstatus = BLOCKED_STATE;
        setcontext(&(readythdinfo->threadcontext));	
    }
    
    interrupts_set(enabled);
    return readyidinfo;
}

int thread_wakeup(struct wait_queue *waitQueue, int isall) 
{
	 // waitQueue is null or theres nothing in the wait waitQueue
    if(NULL != waitQueue  && NULL != waitQueue->curwaitqueue && NULL != waitQueue->curwaitqueue->nextnode) 
    {
        int enabled = interrupts_set(0);
        int cnt = 0;

        if(isall) 
        {
            Tid curid = GetFrontQueueData(waitQueue->curwaitqueue);
            while(curid != THREAD_NONE) 
            {
                cnt++;
                DelFromQueue(waitQueue->curwaitqueue, curid); // take it out of the wait waitQueue
                AddDataToQueue(readyQueue, threadinfos[curid]);            
                threadinfos[curid]->thdstatus = READY_STATE;
                curid = GetFrontQueueData(waitQueue->curwaitqueue);
            }

            interrupts_set(enabled);
            return cnt;
        }
        else 
        {
            Tid curid = GetFrontQueueData(waitQueue->curwaitqueue);
            if(THREAD_NONE != curid) 
            {
                DelFromQueue(waitQueue->curwaitqueue, curid); // take it out of the wait waitQueue
                AddDataToQueue(readyQueue, threadinfos[curid]);
                threadinfos[curid]->thdstatus = READY_STATE;
            }

            interrupts_set(enabled);
            return 1;
        }

        interrupts_set(enabled);
    }

	return 0;
}

Tid thread_wait(Tid tid, int *exit_code) 
{
    if(!(tid >= 0 && tid < THREAD_MAX_THREADS) && (NULL != threadinfos[tid]) && (tid != thread_id())) 
    {   
        return THREAD_INVALID;
    }

    int enabled = interrupts_set(0);
    // printf("thread_wait tid=%d\n", tid);
    thread_sleep(threadinfos[tid]->waitQueue); 
    if (exit_code!=NULL)
    {
        // int dd = threadinfos[tid]->exitcode;
        // printf("tid=%d dd=%d\n", tid, dd);
        //  printf("dd=%lu\n", (unsigned long)(int *)threadinfos[tid]);
    }
        
        // *exit_code = threadinfos[tid]->exitcode;
    interrupts_set(enabled);
	return tid;
}

struct lock 
{
	struct wait_queue* waitQueue;
    Tid thread;
    int id;
};

struct lock* lock_create() 
{
    int enabled = interrupts_set(0);
	struct lock *lockinfo;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	lockinfo = malloc(sizeof(struct lock));
    lockinfo->waitQueue = wait_queue_create();
    lockinfo->id = 0;

    interrupts_set(enabled);
	return lockinfo;
}

void lock_destroy(struct lock *lockinfo) 
{
    int enabled = interrupts_set(0);
	if(0 == lockinfo->id) 
    {
        wait_queue_destroy(lockinfo->waitQueue);
        FREEIF(lockinfo);
    }

    interrupts_set(enabled);
}

void lock_acquire(struct lock *lockinfo) 
{
    int enabled = interrupts_set(0);

    while(1 == lockinfo->id) 
    {
        thread_sleep(lockinfo->waitQueue);
    }

    lockinfo->id = 1;
    lockinfo->thread = thread_id();
    
    interrupts_set(enabled);
}

void lock_release(struct lock *lockinfo) 
{
    int enabled = interrupts_set(0);
    
    if(1 == lockinfo->id && thread_id() == lockinfo->thread) 
    {
        lockinfo->id = 0;
        thread_wakeup(lockinfo->waitQueue, 1);
    }

    interrupts_set(enabled);
}

struct cv 
{
	struct wait_queue *waitQueue;
};

struct cv *cv_create()
{
    int enabled = interrupts_set(0);
	struct cv *cvinfo = malloc(sizeof(struct cv));
    cvinfo->waitQueue = wait_queue_create();
	
    interrupts_set(enabled);
	return cvinfo;
}

void cv_destroy(struct cv *cvinfo)
{
    int enabled = interrupts_set(0);
	
    if(NULL == cvinfo->waitQueue->curwaitqueue->nextnode) 
    {
        wait_queue_destroy(cvinfo->waitQueue);
	    FREEIF(cvinfo);
    }
    
    interrupts_set(enabled);
}

void cv_wait(struct cv *cvinfo, struct lock *lockinfo)
{
    int enabled = interrupts_set(0);
    if(thread_id() == lockinfo->thread) 
    {
        lock_release(lockinfo);
        thread_sleep(cvinfo->waitQueue);
    }

    lock_acquire(lockinfo);
    interrupts_set(enabled);
}

void
cv_signal(struct cv *cvinfo, struct lock *lockinfo)
{
    int enabled = interrupts_set(0);
    if(thread_id() == lockinfo->thread) 
        thread_wakeup(cvinfo->waitQueue, 0);
    
    interrupts_set(enabled);
}

void
cv_broadcast(struct cv *cvinfo, struct lock *lockinfo)
{
	int enabled = interrupts_set(0);

    if(lockinfo->thread == thread_id()) 
        thread_wakeup(cvinfo->waitQueue, 1);
    

    interrupts_set(enabled);
}



/////////////////HELPER FUNCTIONS///////////////////////////////////



Tid GetAvailId () 
{
    for(int i = 0; i < THREAD_MAX_THREADS; i++) 
    {       
        if (NULL == threadinfos[i] || (RUNNING_STATE != threadinfos[i]->thdstatus  && READY_STATE != threadinfos[i]->thdstatus  && BLOCKED_STATE != threadinfos[i]->thdstatus))
            return i;
    }
        
    return THREAD_NOMORE;
}

Tid GetFrontQueueData(StQueue* waitQueue) 
{     
    if(NULL != waitQueue && NULL != waitQueue->nextnode)
    {
        int id = waitQueue->nextnode->threadinfo->threadid;
        return id; 
    } 						                    
    else
        return THREAD_NONE; 			
}

void AddDataToQueue(StQueue *waitQueue, StThreadInfo *threadinfo) 
{
    if(NULL == waitQueue) 		
        return; 

    StQueue *newthdinfo  = (StQueue *)malloc(sizeof(StQueue));
    // initialize new node in waitQueue
    newthdinfo->threadinfo = threadinfo;
    newthdinfo->nextnode = NULL;
    
    if(NULL == waitQueue->nextnode) 
        waitQueue->nextnode = newthdinfo;
    else 
    { 
		StQueue *tempinfo = waitQueue;
		while(NULL != tempinfo->nextnode) 
		    tempinfo = tempinfo->nextnode;
		tempinfo->nextnode = newthdinfo;
    }
}

void DelFromQueue(StQueue* waitQueue, Tid threadid) 
{
    if(NULL == waitQueue || NULL == waitQueue->nextnode) 		
        return; 

    StQueue *previnfo = waitQueue;
    StQueue *currinfo = previnfo->nextnode; // first element of the list

    while(threadid != currinfo->threadinfo->threadid && NULL != currinfo->nextnode) 
    {
        previnfo = currinfo;
        currinfo = currinfo->nextnode;
    }

    if(threadid == currinfo->threadinfo->threadid) 
    {
        previnfo->nextnode = currinfo->nextnode;
        FREEIF(currinfo);
    }
}

void DelAllQueueData(Tid threadid) 
{
    for(int i = 0; i < THREAD_MAX_THREADS; i++) 
    {
        if(NULL != threadinfos[i]) 
        {
            StQueue *currinfo = threadinfos[i]->waitQueue->curwaitqueue;
            if(NULL != currinfo->nextnode) 
            {
                currinfo = currinfo->nextnode;
            }
            
            while(currinfo != NULL) 
            {
                if(NULL != currinfo->threadinfo && threadid == currinfo->threadinfo->threadid) 
                {  
                    DelFromQueue(threadinfos[i]->waitQueue->curwaitqueue, threadid);
                }
                
                currinfo = currinfo->nextnode;
            }
        }
    }
}

void FreeThreadData(Tid threadid) 
{
    FREEIF(threadinfos[threadid]->stackpoint);	
    wait_queue_destroy(threadinfos[threadid]->waitQueue);			
    FREEIF(threadinfos[threadid]);
}

Tid FreeExitedThd() 
{    
    for(Tid i = 0; i < THREAD_MAX_THREADS; i++) 
    {
        if(NULL != threadinfos[i]  && EXITED_STATE == threadinfos[i]->thdstatus) 
        {
            DelFromQueue(readyQueue, i);
            FreeThreadData(i);
            return i;
        }

    }

    return THREAD_NONE;
}

// yield to another threadinfo from thread_exit
Tid YieldThd(int code) 
{   
    Tid ready_id = GetFrontQueueData(readyQueue); 					// get the first threadinfo from ready waitQueue to yield to
    if(THREAD_NONE == ready_id)	
    {
        return ready_id; 			// if the only threadinfo left is the one running, exit the program
    }

    threadinfos[ready_id]->thdstatus = RUNNING_STATE;
    threadinfos[ready_id]->exitcode = code;
    DelFromQueue(readyQueue, threadinfos[ready_id]->threadid);	// delete ready threadinfo form ready waitQueue	
    setcontext(&(threadinfos[ready_id]->threadcontext));	

    return ready_id;
}


