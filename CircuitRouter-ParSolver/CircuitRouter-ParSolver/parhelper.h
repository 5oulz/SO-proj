#ifndef PARHELPER_H
#define PARHELPER_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

extern pthread_mutex_t queueLock;

void initLock(pthread_mutex_t lock);
void destroyLock(pthread_mutex_t lock);

#endif /* PARHELPER_H */