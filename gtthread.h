#ifndef GTTHREAD_H
#define GTTHREAD_H

#include "steque.h"
#include <ucontext.h>


/* Define gtthread_t and gtthread_mutex_t types here */

sigset_t vtalrm;             // signal set

#define SUCCESS 0
#define FAIL 1   

typedef long gtthread_t;

typedef enum state{ // thread state 
  READY,
  RUNNING,
  TERMINATED,
  JOINED
} gtthread_state;

typedef struct mutex_b_t{
  int lock;
  steque_t wait_queue;
} gtthread_mutex_t;

typedef struct thread_b_t{ // thread data structure
  ucontext_t uctx;
  gtthread_t tID;
  gtthread_state state;
  void *retval;
} gtthread_blk_t;

gtthread_blk_t *get_cur_thread(void);

void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
int  gtthread_mutex_destroy(gtthread_mutex_t *mutex);
#endif
