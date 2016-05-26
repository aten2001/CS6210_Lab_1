# CS6210_Lab_1
GT thread (individual assignment)

## Objective
An user-level thread API package which is similar to the Pthread is implemented in this project. Besides, the classic Dining Philosopher problem is implemented using the GT thread.

## API description  
#### general
- void gtthread_init(long period);
 - The function must be called by the main thread before other APIs are called (assumption). The caller can specify the scheduling period via this initialization function.
- int  gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg);  
 - A GT thread is created by this function. The context is created by using *u_context* which is provided by c library. The neccessary parameters are set in this function. 
- int  gtthread_join(gtthread_t thread, void **status);  
 - The function is analogous to *pthread_join()*. All the threads are joinable in this project. The function should check the status of the join thread (terminated or not) and yield to other thread if the thread is not terminated yet.
- void gtthread_exit(void *retval);  
 - The function is the same as *pthread_exit()*. The thread state is changed to *TERMINATED* and the return value is stored to the thread block. 
- int  gtthread_yield(void);
 - The yield function is the same as that in pthread. When this function is called, it releases its ownership and force the alarm handler to execute (details described later). 
- int  gtthread_equal(gtthread_t t1, gtthread_t t2);  
 - Returning zero if the two threads t1 and t2 have the same thread ID.
- int  gtthread_cancel(gtthread_t thread);
 - Allowing one thread to terminate other threads asynchronously by specifying the thread ID.
- gtthread_t gtthread_self(void);
 - Returning the thread ID of the calling thread. 

#### mutex
- int  gtthread_mutex_init(gtthread_mutex_t *mutex);
 - It is analogous to *pthread_mutex_init()*. The wait queue for the lock is initialized here. 
- int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
 - This function is called when the lock is acquired. If the lock is already got by other thread, then the thread will be added to the wait queue and yield to other threads until the lock is released and the top of the wait queue is the thread.
- int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
 - The function is called when the lock is going to be released. 

## Notes
#### scheduler
In this project, the scheduler is round-robin based. Therefore, I use a queue data structure and timer to schedule the threads. For a given time slot, there will be a thread scheduled at the time slot. When time is up, the alarm handler will be serviced to update the thread status (READY, RUNNING, JOINED, and TERMINATED) and update the elements in the scheduling queue. 

#### queue
There is a package of basic queue functions provided by the instructors. There are two queues used in this project: 
1. scheduling queue: stores the threads that to be scheduled.
2. wait queue for a lock: stores the threads that have acquired the lock

#### block signal
Note that for some of the APIs should be atomic. In other words, the timer signal should not interrupt the executing APIs. Therefore, we use the following function calls to disable/enable the alarm signal: 
```
sigprocmask(SIG_BLOCK, &vtalrm, NULL);                        // disable alarm signal
sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);                      // enable alarm signal
```
