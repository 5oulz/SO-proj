#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t queueLock;


/** defined here in case there's a need to perform aditional operations over the locks */
/** inits given lock **/
void initLock(pthread_mutex_t lock) {
    if( pthread_mutex_init(&lock, NULL) != 0 ) {
        puts("Mutex init failed");
        exit(1);
    } else {
        //puts("lock up");
    }
}

/** destroys given lock **/
void destroyLock(pthread_mutex_t lock) {
    //puts("lock down");
    pthread_mutex_destroy(&lock);
}
