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
#define PERMS 0644

struct timer {
	unsigned int ns;
        unsigned int seconds;
};

struct message {
	long mtype;
	int mint;
	pid_t pid;
};

struct page {
	int id;
	int valid;
	int dirty;
	int readOrWrite;
};

key_t keyTime = 1234;
key_t keyChild = 4321;
key_t keyTerm = 1324;
key_t keyRes = 4231;
int errno;
int shmidC, shmidT, shmidR, shmidTime;
struct timer *shmTimer;
int *shmC;
int *shmT;
struct page *shmP;
sem_t * semD;
sem_t * semT;
sem_t * semC;
int lockProc[18] = {0};
int logCount = 0;
int totLocked = 0;

#endif
