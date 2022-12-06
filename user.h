#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#define THRESHOLD 20
#define BOUND 500

struct timer {
	unsigned int ns;
	unsigned int seconds;
};

struct message {
	long mtype;
	int mint;
};

struct page {
	int id;
	int valid;
	int dirty;
	int readOrWrite;
};

struct timer *shmTimer;
struct page *shmP;
int *shmC;
int *shmT;
sem_t * semD;
sem_t * semT;
sem_t * semC;
int errno, myIndex;

#endif
