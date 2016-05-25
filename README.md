# CS6210_Lab_1
GT thread (individual assignment)

## Objective
An user-level thread API package which is similar to the Pthread is implemented in this project. Besides, the classic Dining Philosopher problem is implemented using the GT thread.

## API description  
- void gtthread_init(long period);
- int  gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg);  
- int  gtthread_join(gtthread_t thread, void **status);  
- void gtthread_exit(void *retval);  
- int  gtthread_yield(void);
- int  gtthread_equal(gtthread_t t1, gtthread_t t2);
- int  gtthread_cancel(gtthread_t thread);
- gtthread_t gtthread_self(void);
- int  gtthread_mutex_init(gtthread_mutex_t *mutex);
- int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
- int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);

