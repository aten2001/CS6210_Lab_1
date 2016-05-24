#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>

#include "philosopher.h"

static pthread_mutex_t chop_locks[5];
#define LEFT 0
#define RIGHT 1

/*
 * Performs necessary initialization of mutexes.
 */
void chopsticks_init(){
    int i;

    for(i=0; i<5; i++)
        pthread_mutex_init(&chop_locks[i], NULL);
 
}
/*
 * Cleans up mutex resources.
 */
void chopsticks_destroy(){

    int i;

    for(i=0; i<5; i++)
        pthread_mutex_destroy(&chop_locks[i]);

}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void pickup_chopsticks(int phil_id){

    if(get_chop_index(phil_id, LEFT) > get_chop_index(phil_id, RIGHT)){
        pthread_mutex_lock(&chop_locks[get_chop_index(phil_id, LEFT)]);
        pickup_left_chopstick(phil_id);
        pthread_mutex_lock(&chop_locks[get_chop_index(phil_id, RIGHT)]);
        pickup_right_chopstick(phil_id);
    }
    else{
        pthread_mutex_lock(&chop_locks[get_chop_index(phil_id, RIGHT)]);
        pickup_right_chopstick(phil_id);
        pthread_mutex_lock(&chop_locks[get_chop_index(phil_id, LEFT)]);
        pickup_left_chopstick(phil_id);    
    }  

}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void putdown_chopsticks(int phil_id){

    if(get_chop_index(phil_id, LEFT) > get_chop_index(phil_id, RIGHT)){
        putdown_right_chopstick(phil_id);
        pthread_mutex_unlock(&chop_locks[get_chop_index(phil_id, RIGHT)]);
        putdown_left_chopstick(phil_id);
        pthread_mutex_unlock(&chop_locks[get_chop_index(phil_id, LEFT)]);
        
    }
    else{
        putdown_left_chopstick(phil_id);
        pthread_mutex_unlock(&chop_locks[get_chop_index(phil_id, LEFT)]);
        putdown_right_chopstick(phil_id);
        pthread_mutex_unlock(&chop_locks[get_chop_index(phil_id, RIGHT)]);
        
    }
}


int get_chop_index(int phil_id, int right){

    if(right)
        return (phil_id);

    return ((phil_id+4)%5);
}