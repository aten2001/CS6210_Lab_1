/**********************************************************************
gtthread_mutex.c.  

This file contains the implementation of the mutex subset of the
gtthreads library.  The locks can be implemented with a simple queue.
 **********************************************************************/

/*
  Include as needed
*/


#include "gtthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG_ON


#ifdef DEBUG_ON
#define DEBUG_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...) 
#endif

/*
  The gtthread_mutex_init() function is analogous to
  pthread_mutex_init with the default parameters enforced.
  There is no need to create a static initializer analogous to
  PTHREAD_MUTEX_INITIALIZER.
 */
int gtthread_mutex_init(gtthread_mutex_t* mutex){

  DEBUG_MSG("gtthread_mutex_init:\n");
  steque_init(&mutex->wait_queue);
  mutex->lock = 0;
  sigemptyset(&vtalrm);
  sigaddset(&vtalrm, SIGVTALRM);

  return SUCCESS;

}

/*
  The gtthread_mutex_lock() is analogous to pthread_mutex_lock.
  Returns zero on success.
 */
int gtthread_mutex_lock(gtthread_mutex_t* mutex){

  gtthread_blk_t *curr, *head;

  if(mutex->lock == 0){
      DEBUG_MSG("gtthread_mutex_lock\n");
      mutex->lock = 1;
  }
  else{  
    sigprocmask(SIG_BLOCK, &vtalrm, NULL);     
    curr = get_cur_thread();
    steque_enqueue(&mutex->wait_queue, curr);

    DEBUG_MSG("curr->tID=%ld\n", curr->tID);
    sigprocmask(SIG_UNBLOCK, &vtalrm, NULL); 
    gtthread_yield();

    head = steque_front(&mutex->wait_queue);

    while((mutex->lock) || (head->tID != curr->tID)){
      gtthread_yield();
      //DEBUG_MSG("locked!\n");
      head = steque_front(&mutex->wait_queue);
      //DEBUG_MSG("head->tID=%ld\n", head->tID);
      
    }

    DEBUG_MSG("gtthread_mutex_lock: pop from wait queue,tID:%d\n", curr->tID);
    steque_pop(&mutex->wait_queue);
    mutex->lock = 1;

  }

  return SUCCESS;
}

/*
  The gtthread_mutex_unlock() is analogous to pthread_mutex_unlock.
  Returns zero on success.
 */
int gtthread_mutex_unlock(gtthread_mutex_t *mutex){

  sigprocmask(SIG_BLOCK, &vtalrm, NULL); 
  if(mutex->lock){
    DEBUG_MSG("gtthread_mutex_unlock");
    mutex->lock = 0;
  }

  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL); 
  return SUCCESS;

}

