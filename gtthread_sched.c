/**********************************************************************
gtthread_sched.c.  

This file contains the implementation of the scheduling subset of the
gtthreads library.  A simple round-robin queue should be used.
 **********************************************************************/
/*
  Include as needed
*/

#include "gtthread.h"

#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
   Students should define global variables and helper functions as
   they see fit.
 */
//#define DEBUG_ON

static steque_t thread_queue;
static struct itimerval timer;      // timer stucture
static struct sigaction alarm_sig;  // alarm stucture 

static gtthread_t thread_ID = 1;
static long quantum = 0;

#ifdef DEBUG_ON
#define DEBUG_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...) 
#endif
void alarm_handler(int s);


gtthread_blk_t *get_cur_thread(){

  return (gtthread_blk_t *)steque_front(&thread_queue);

}

/* Timer initialization */

void timer_init(long quan){

  quantum = quan;

  memset(&alarm_sig, 0, sizeof(alarm_sig));
  alarm_sig.sa_handler = &alarm_handler;
  sigaction(SIGVTALRM, &alarm_sig, NULL);

  sigemptyset(&vtalrm);
  sigaddset(&vtalrm, SIGVTALRM);

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = quantum;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = quantum;

  setitimer(ITIMER_VIRTUAL, &timer, NULL);

}

void reset_timer(){

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = quantum;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = quantum;

  setitimer(ITIMER_VIRTUAL, &timer, NULL);

}


void thread_handler(void *(*thread_func)(void *), void *arg){

  void *retval;
  gtthread_blk_t *cur;
  cur = get_cur_thread();
  DEBUG_MSG("thread_handler: thread ID: # %ld\n", cur->tID);
  
  retval = (*thread_func)(arg);
  gtthread_exit(retval);
  

}

void alarm_handler(int sig){

  gtthread_blk_t *cur, *next;

  DEBUG_MSG("alarm_handler\n");

  cur = (gtthread_blk_t *)steque_front(&thread_queue);
  if(cur->state == RUNNING){                                       // avoid update state when it's already done
    cur->state = READY;
    steque_cycle(&thread_queue);
  }

  /*if((cur->state == TERMINATED) && (cur->tID == 1)){

    while(!steque_isempty(&thread_queue)){
      steque_pop(&thread_queue);
    }

    return;

  } */

  while((next = (gtthread_blk_t *)steque_front(&thread_queue))){ 
  
    if(next->state == READY){
      next->state = RUNNING;
      DEBUG_MSG("Schedule #%ld thread\n", next->tID);
      break;
    }
    else if(next->state == JOINED){
      steque_pop(&thread_queue);
      DEBUG_MSG("POP #%ld thread\n", next->tID);
    }
    else if(next->state == TERMINATED)
      steque_cycle(&thread_queue);
    else{
      steque_pop(&thread_queue);
      DEBUG_MSG("ERROR! RUNNING thread?\n");
    }
  } 

  DEBUG_MSG("swap: cur thread: # %ld, next thread: # %ld\n", cur->tID, next->tID);
  reset_timer();

  if (swapcontext(&cur->uctx, &next->uctx) != 0)
    DEBUG_MSG("ERROR! Swap failed?\n");
  DEBUG_MSG("GOOD! Swap SUCCESS!\n");

  list_thread();

}

gtthread_blk_t *get_thread(gtthread_t tID){

  int queue_len, i;
  gtthread_blk_t *tmp, *ret;
  int match = 0;

  DEBUG_MSG("get_thread, tID: %ld\n", tID);

  queue_len = steque_size(&thread_queue);
  
  for(i = 0; i < queue_len; i++){

    tmp = (gtthread_blk_t *)steque_pop(&thread_queue);
    steque_enqueue(&thread_queue, tmp);

    if(tmp->tID == tID){
      ret = tmp;
      match = 1;
    } 

  }

  if(!match)
    DEBUG_MSG("No Match!\n");

  return ret;

}

void list_thread(){

  int queue_len, i;
  gtthread_blk_t *tmp; 

  DEBUG_MSG("----------------------------------------------\n");

  queue_len = steque_size(&thread_queue);
  
  for(i = 0; i < queue_len; i++){

    tmp = (gtthread_blk_t *)steque_pop(&thread_queue);

    DEBUG_MSG("tmp->tID: %ld, state: %d, context: %x\n", tmp->tID, tmp->state, &tmp->uctx);

    steque_enqueue(&thread_queue, tmp);

  }
  DEBUG_MSG("----------------------------------------------\n");

}

/*
  The gtthread_init() function does not have a corresponding pthread equivalent.
  It must be called from the main thread before any other GTThreads
  functions are called. It allows the caller to specify the scheduling
  period (quantum in micro second), and may also perform any other
  necessary initialization.  If period is zero, then thread switching should
  occur only on calls to gtthread_yield().

  Recall that the initial thread of the program (i.e. the one running
  main() ) is a thread like any other. It should have a
  gtthread_t that clients can retrieve by calling gtthread_self()
  from the initial thread, and they should be able to specify it as an
  argument to other GTThreads functions. The only difference in the
  initial thread is how it behaves when it executes a return
  instruction. You can find details on this difference in the man page
  for pthread_create.
 */
void gtthread_init(long period){

  ucontext_t uctx_main;
  gtthread_blk_t *main_thread;

  /* Initialize the thread queue */
  steque_init(&thread_queue);
    
  //fprintf(stderr, "gtthread_init\n"); 
  DEBUG_MSG("gtthread_init\n");

  /* Create main thread and put into queue */

  /************************* Initialize the context *************************************/

  if(getcontext(&uctx_main) == -1){ 
    printf("getcontext FAIL!\n");
    return;
  }

  uctx_main.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  uctx_main.uc_stack.ss_size = SIGSTKSZ;
  uctx_main.uc_link = NULL;

  //makecontext(&uctx_main, thread_handler, 0);      // modify the context 

  /************************** Initialize the thread block *************************************/

  if(!(main_thread = malloc(sizeof(gtthread_blk_t)))){
    printf("main_thread created failed\n");
    return;
  }
  main_thread->tID = thread_ID++;
  main_thread->state = RUNNING;
  main_thread->uctx = uctx_main;

  /********************************************************************************************/

  steque_enqueue(&thread_queue, main_thread);                     // add the thread to the scheduler queue

  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                        // enable alarm signal


  /* Initialize timer and alarm */
  timer_init(period);


}


/*
  The gtthread_create() function mirrors the pthread_create() function,
  only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread,
        void *(*start_routine)(void *),
        void *arg){

  ucontext_t uctx;
  gtthread_blk_t *th_blk;

  DEBUG_MSG("gtthread_create\n"); 

  //sigprocmask(SIG_BLOCK, &vtalrm, NULL);                          // disable alarm signal

  /************************* Initialize the context *************************************/


  if(getcontext(&uctx) == -1) 
    return FAIL;

  uctx.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  uctx.uc_stack.ss_size = SIGSTKSZ;
  uctx.uc_link = NULL;

  makecontext(&uctx, (void (*) (void))(thread_handler), 2, start_routine, arg);      // modify the context 


  /************************** Initialize the thread block *************************************/

  th_blk = (gtthread_blk_t *)malloc(sizeof(gtthread_blk_t));
  if(th_blk == NULL)
    return FAIL;

  th_blk->tID = thread_ID++;
  th_blk->state = READY;
  th_blk->uctx = uctx;

  if(th_blk->uctx.uc_stack.ss_size == SIGSTKSZ)
    DEBUG_MSG("Correct assign\n"); 

  /********************************************************************************************/

  steque_enqueue(&thread_queue, th_blk);                          // add the thread to the scheduler queue

  //sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                        // enable alarm signal

  *thread = th_blk->tID;

  //list_thread();
  gtthread_yield();

  return SUCCESS;
}

/*
  The gtthread_join() function is analogous to pthread_join.
  All gtthreads are joinable.
 */
int gtthread_join(gtthread_t thread, void **status){

  gtthread_blk_t *join;

  DEBUG_MSG("gtthread_join\n"); 

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);                        // disable alarm signal
  join = get_thread(thread);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                      // enable alarm signal

  if(join == NULL)
    return FAIL;

  while(join->state != TERMINATED){
    DEBUG_MSG("Thread # %ld join: not TERMINATED!\n", join->tID); 
    gtthread_yield();
  }

  DEBUG_MSG("Thread # %ld join: TERMINATED!\n", join->tID); 
  //sigprocmask(SIG_BLOCK, &vtalrm, NULL);                        // disable alarm signal

  join->state = JOINED;
  if (status != NULL)
    *status = join->retval;

  //sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                        // enable alarm signal

  return SUCCESS;

}

/*
  The gtthread_exit() function is analogous to pthread_exit.
 */
void gtthread_exit(void* retval){

  gtthread_blk_t *term;

  DEBUG_MSG("gtthread_exit\n");

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);                        // disable alarm signal

  term = (gtthread_blk_t *)steque_front(&thread_queue);
  if(term->state != RUNNING)
  {
    DEBUG_MSG("ERROR state when gtthread_exit: term->state != RUNNING\n");
    return;
  }

  term->state = TERMINATED;
  term->retval = retval;

  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                        // enable alarm signal
  gtthread_yield();

}


/*
  The gtthread_yield() function is analogous to pthread_yield, causing
  the calling thread to relinquish the cpu and place itself at the
  back of the schedule queue.
 */
void gtthread_yield(void){

  raise(SIGVTALRM);

}

/*
  The gtthread_yield() function is analogous to pthread_equal,
  returning zero if the threads are the same and non-zero otherwise.
 */
int gtthread_equal(gtthread_t t1, gtthread_t t2){

  return (t1 == t2);
}

/*
  The gtthread_cancel() function is analogous to pthread_cancel,
  allowing one thread to terminate another asynchronously.
 */
int gtthread_cancel(gtthread_t thread){

  gtthread_blk_t *cancel;

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);                        // disable alarm signal

  cancel = get_thread(thread);
  if((cancel == NULL) || (cancel->state == JOINED))
    return FAIL;

  cancel->state = TERMINATED;
  
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                      // enable alarm signal
  
  return SUCCESS;

}

/*
  Returns calling thread.
 */
gtthread_t gtthread_self(void){

  gtthread_blk_t *self;

  sigprocmask(SIG_BLOCK, &vtalrm, NULL);                        // disable alarm signal

  self = (gtthread_blk_t *)steque_front(&thread_queue);

  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                      // enable alarm signal

  return self->tID;



}
