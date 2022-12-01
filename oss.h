#ifndef OSS_H
#define OSS_H

#include <semaphore.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define DDA 100000

struct timer {
	unsigned int ns;
        unsigned int seconds;
};

struct resource {
	unsigned int reqArray[18];
        unsigned int allArray[18];
        unsigned int relArray[18];
        
	int shared;
        unsigned int allocation;
        unsigned int release;	
	unsigned int maxAmt;
        unsigned int available;
        unsigned int request;
};

FILE *out_file;
key_t keyTime = 1234;
key_t keyChild = 4321;
key_t keyTerm = 1324;
key_t keyRes = 4231;
int errno;
int shmidC, shmidT, shmidR, shmidTime;
struct timer *shmTimer;
int *shmC;
int *shmT;
struct resource *shmR;
sem_t * semD;
sem_t * semT;
sem_t * semC;
int lockProc[18] = {0};
int logCount = 0;
int totLocked = 0;

#endif
