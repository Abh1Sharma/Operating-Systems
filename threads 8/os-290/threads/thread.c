#include "thread.h"
#include "interrupt.h"
#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
//#include "lab3.h"
#define NTHREADS   128
#define FREEIF(x) if (x) \
{ \
    free(x); \
    x = NULL; \
}
// this variable Free define is from geeksforgeeks, used for freeing and in lab23
typedef enum  
{
    BLOCKED_STATE = 0,
    EXITED_STATE = 1,
    RUNNING_STATE =2,
    READY_STATE = 3
}thread_state;
// val add is irrel
/* This is the thread control block */
typedef struct  
{
	/* ... Fill this in ... */
        void *stackpoint; 
        //stp points to pos in arm
        ucontext_t threadcontext;
        // context struct ptr
        
        thread_state thd_state;
        
	Tid threadid;
		
}StThreadInfo;

// implementing queue w ptr to next and thread info strct
typedef struct QueueNode 
{
    struct QueueNode * nextnode;
    StThreadInfo threadinfo;
}StQueueNode;

/* This is the wait queue structure */
struct wait_queue 
{
    // sep file imple
	// /* ... Fill this in Lab 3 ... */
	
};
/// declaring struct ptrss
StQueueNode *rearExitQueue;
StQueueNode *exitQueue;
StQueueNode *currentQueue;
StQueueNode *readyQueue;
Tid thd_num;
StQueueNode *rearReadyQueue;
Tid killThdid[THREAD_MAX_THREADS];
int killthdnum;
// ptr strcut funct o create queue, lab3use
struct wait_queue *wait_queue_create();

struct lock {
	/* ... Fill this in ... */
	
};

StQueueNode *GetMsgFromQueue(StQueueNode **curnode, StQueueNode **rearnode, Tid threadid) 
{
	
    StQueueNode * preNode = NULL;
    StQueueNode * tmpcurcode = *curnode;
    while (tmpcurcode != NULL) 
	{
        if (tmpcurcode->threadinfo.threadid != threadid) 
	{
            preNode = tmpcurcode;
            tmpcurcode = tmpcurcode->nextnode;
        }
        else
            {
            if (preNode == NULL) 
            {
                if (tmpcurcode == *rearnode)
                {
                    *rearnode = tmpcurcode->nextnode;
                }
            *curnode = tmpcurcode->nextnode;
            } 
            else 
            {
                preNode->nextnode = tmpcurcode->nextnode;
                if (tmpcurcode == *rearnode){
                    *rearnode = preNode;   
                } 
					
            }

            return tmpcurcode;
            }
        
    }

    return NULL;
}

void thread_init(void)
{
	/* your optional code here */
    thd_num = 1;
    currentQueue = malloc(sizeof(StQueueNode));
    
    currentQueue->threadinfo.stackpoint = NULL;
    //
    currentQueue->nextnode = NULL;
    currentQueue->threadinfo.threadid = 0;
    currentQueue->threadinfo.thd_state = RUNNING_STATE;
    //
    rearReadyQueue = NULL;
    exitQueue = NULL;	
    killthdnum = 0;
    //
    readyQueue = NULL;
    rearExitQueue = NULL;
}

Tid thread_id()
{
    int id = currentQueue->threadinfo.threadid;	
    return id;
}

/* New thread starts by calling thread_stub. The arguments to thread_stub are
 * the thread_main() function, and one argument to the thread_main() function. 
 */
void
thread_stub(void (*thread_main)(void *), void *arg)
{
//    printf("argP: %p", arg);
    thread_main(arg); // call thread_main() function with arg
    thread_exit(0);
        
}
/// to implement que we need to check for empty nulls, do the temp switch replace and check others
/// TODO clear clutter COMMENTS
Tid PushMsgToQueue(StQueueNode **curnode, StQueueNode **rearnode, StQueueNode *tmpnode) 
{
	if (*curnode)
	{
            (*rearnode)->nextnode = tmpnode;
            
            (*rearnode) = tmpnode;
            
	}
	else
	{
            *curnode = tmpnode;
            //printf()
            (*rearnode) = tmpnode;
            //
	}
    
    return tmpnode->threadinfo.threadid;
}
/// TODO clear clutter COMMENTS
Tid thread_create(void (*fn) (void *), void *parg)
{
//	setting, clearing all var and stack state
	if (thd_num >= THREAD_MAX_THREADS ) 
	{
            return THREAD_NOMORE;
	}
	
	ucontext_t curContext;
	getcontext(&curContext);
	
	StQueueNode * curnode = (StQueueNode *)malloc(sizeof(StQueueNode));
	if (curnode == NULL) 
	{
            return THREAD_NOMEMORY;
	}

	curContext.uc_stack.ss_sp = malloc(THREAD_MIN_STACK);
	if (curContext.uc_stack.ss_sp == NULL) 
	{
	    FREEIF(curnode);
		
	    return THREAD_NOMEMORY;
	}
//        setting, clearing all var and stack state
	curContext.uc_stack.ss_size = THREAD_MIN_STACK;
	curContext.uc_stack.ss_flags= 0;
	
	unsigned long int * stackpt = (unsigned long int *)((unsigned long int)curContext.uc_stack.ss_sp + (unsigned long int)curContext.uc_stack.ss_size);
//        printf("stackPtr:%d",);
//        setting stack ptr to be at address of -8bytes in stack, replace and push later
	stackpt = (unsigned long int *)(((unsigned long int)stackpt& -16L) - 8);
	//REG_RIP is The entry function of the context
//        printf("stackPtr:%d",);
	curContext.uc_mcontext.gregs[REG_RIP] = (unsigned long int)thread_stub;
//        printf("stackPtr:%d",);
	//REG_RDI save calling thread function point
	curContext.uc_mcontext.gregs[REG_RDI] = (unsigned long int)fn;
//       printf("stackPtr:%d",);
	//REG_RSI is ave thread param
	curContext.uc_mcontext.gregs[REG_RSI] = (unsigned long int)parg;
	//sp is Stack pointer
//        printf("stackPtr:%d",);
	curContext.uc_mcontext.gregs[REG_RSP] = (unsigned long int)stackpt;
//        printf("stackPtr:%d",);

	if (killthdnum == 0)
        {
            curnode->threadinfo.threadid = thd_num;
            thd_num++;
            //printf("num: %d",thdnum);
        }
	else 
	{
            curnode->threadinfo.threadid = killThdid[killthdnum-1];
//                printf("curr: ");
            killthdnum--;
//                printf("kill num: %d",killthdnum);
            thd_num++;
	}

	curnode->threadinfo.thd_state = READY_STATE;
	curnode->threadinfo.threadcontext = curContext;
	curnode->threadinfo.stackpoint = curContext.uc_stack.ss_sp;
        // set next node to null cause gonna call create again next time
	curnode->nextnode = NULL;
	 
	PushMsgToQueue(&readyQueue, &rearReadyQueue, curnode);
	
	return curnode->threadinfo.threadid;
}
/// TODO clear clutter COMMENTS
void exitthdfromqueue() 
{
	StQueueNode *tmpnode = exitQueue;
	while (tmpnode != NULL) 
	{
            if (tmpnode->threadinfo.threadid == 0) 
            {
                tmpnode = tmpnode->nextnode;
                continue;
            }

		
            StQueueNode * tmpcode = GetMsgFromQueue(&tmpnode, &rearExitQueue, tmpnode->threadinfo.threadid);
		
            tmpcode->nextnode = NULL;
		// get threadid in kill array and free
            killThdid[killthdnum] = tmpcode->threadinfo.threadid;
		
            FREEIF(tmpcode->threadinfo.stackpoint); FREEIF(tmpcode);
            killthdnum++;
            thd_num--;
	}

	exitQueue = NULL;
}

Tid thread_yield(Tid yieldid)
{
	
    exitthdfromqueue();
    int isyield = 0;
    if (yieldid == THREAD_SELF || yieldid == thread_id()) 
	{
//        return thread_id(1);
        return thread_id();
	}

	if (yieldid == THREAD_ANY && readyQueue == NULL) 
	{	
            return THREAD_NONE; 
	}
	else if (yieldid == THREAD_ANY)
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
	if (isyield != 0) 
	{
            return yieldid;
	}
		
//        set isyield to setpoint
	isyield = 1;
	currentQueue->threadinfo.threadcontext = yieldcontext;
	currentQueue->threadinfo.thd_state = READY_STATE;
	
	node->threadinfo.thd_state = RUNNING_STATE;
	PushMsgToQueue(&readyQueue, &rearReadyQueue, currentQueue);
	
	currentQueue = node;
	setcontext(&(currentQueue->threadinfo.threadcontext));			 
		//  thd_num--;
	return yieldid;
}
/// TODO clear clutter COMMENTS
void thread_exit(int exit_code)
{
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

Tid thread_kill(Tid threadid)
{	
    exitthdfromqueue();
    StQueueNode * tmpnode = GetMsgFromQueue(&readyQueue, &rearReadyQueue, threadid);
    if (NULL == tmpnode) 
    {
        return THREAD_INVALID;
    }
	
	// killThdid[killthdnum] = tmpnode->threadinfo.threadid;
	
    tmpnode->threadinfo.thd_state = EXITED_STATE;
    killThdid[killthdnum] = tmpnode->threadinfo.threadid;
    FREEIF(tmpnode->threadinfo.stackpoint);	FREEIF(tmpnode);
	// free stptr and currnode
    killthdnum += 1;
//    printf("killthdnum: %d",killthdnum);
    thd_num--;
	
    return threadid;
}

/// see lab3.c file for separate compilation... cleared work for lab2. 
// ece344-tester 3,2
/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *wait_queue_create()
{
	return NULL;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	// TBD();
	FREEIF(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	// TBD();
	// return THREAD_FAILED;
	return 0;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	
	return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid, int *exit_code)
{
	//TBD();
	return 0;
	
}

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);
	//assert(!lock->state);
	//TBD();

	FREEIF(lock);
}

void
lock_acquire(struct lock *lock)
{
	
	//TBD();
}

void
lock_release(struct lock *lock)
{
	
	// TBD();
}

struct cv {
	/* ... Fill this in ... */
	 
};

struct cv *
cv_create()
{
	// struct cv *cv;
	

	// TBD();

	// return cv;
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	

	assert(cv != NULL);
    // assert(cv->wq->front == NULL);
	//TBD();
   
	
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	// assert(cv != NULL);
	// assert(lock != NULL);

	// TBD();

	
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// assert(cv != NULL);
	// assert(lock != NULL);

	// TBD();
	
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// assert(cv != NULL);
	// assert(lock != NULL);

	// TBD();
	
}
