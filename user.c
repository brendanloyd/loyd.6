#include "user.h"


void sigIntHandler(int signum) {
	
	printf("Child %d: Caught SIGINT! Killing process #%d.\n", getpid(), myIndex);
	
	sem_wait(semT);
	shmT[myIndex] = 1;
	shmT[19]++;
	sem_post(semT);
	
	sem_post(semD);
	
	errno = shmdt(shmTimer);
	if(errno == -1) {
		printf("Child %d: shmdt(shmTimer)\n", getpid());
	}

	errno = shmdt(shmC);
	if(errno == -1) {
		printf("Child %d: shmdt(shmC)\n", getpid());
	}
	
	errno = shmdt(shmT);
	if(errno == -1) {
		printf("Child %d: shmdt(shmT)\n", getpid());
	}
	
	errno = shmdt(shmP);
	if(errno == -1) {
		printf("Child %d: shmdt(shmR)\n", getpid());
	}
	exit(signum);
}

int main (int argc, char *argv[]) {
int i = 0, o = 0, terminate = 0;;
struct timer reqlTime, termTime;
int timeKey = atoi(argv[1]), childKey = atoi(argv[2]), index = atoi(argv[3]);
int termKey = atoi(argv[4]);
int resKey = atoi(argv[5]);
myIndex = index;
signal(SIGINT, sigIntHandler);
unsigned int nextRes;

srand(getpid() * time(NULL));

shmTimer = shmat(timeKey, NULL, 0);
if ((void *)shmTimer == (void *)-1) {
	printf("USER_PROC: shmat(shmidTimer)\n");
    	exit(1);
}

shmC = shmat(childKey, NULL, 0);
if ((void *)shmC == (void *)-1) {
	printf("USER_PROC: shmat(shmidC)\n");
    	exit(1);
}

shmT = shmat(termKey, NULL, 0);
if ((void *)shmT == (void *)-1) {
	printf("USER_PROC: shmat(shmidT)\n");
    	exit(1);
}

//point semaphores to shared memory
shmP = shmat(resKey, NULL, 0);
if ((void *)shmP == (void *)-1) {
	printf("USER_PROC: shmat(shmidRes)\n");
    	exit(1);
}

semD = sem_open("semD", 1);
if(semD == SEM_FAILED) {
	printf("USER_PROC %d: sem_open(semD)\n", getpid());
    	exit(1);
}

semT = sem_open("semT", 1);
if(semT == SEM_FAILED) {
	printf("USER_PROC %d: sem_open(semT)\n", getpid());
    	exit(1);
}

semC = sem_open("semC", 1);
if(semC == SEM_FAILED) {
	printf("USER_PROC %d: sem_open(semC)\n", getpid());
    	exit(1);
}

/* Calculate First Request/Release Time */
reqlTime.ns = shmTimer->ns + rand()%(BOUND);
reqlTime.seconds = shmTimer->seconds;
if (reqlTime.ns > 1000000000) {
	reqlTime.ns -= 1000000000;
	reqlTime.seconds += 1;
}

while(!terminate) {
	if(rand()%100 <= THRESHOLD) {
		terminate = 1;
	}
	
	/* Calculate Termination Time */
	termTime.ns = shmTimer->ns + rand()%250000000;
	termTime.seconds = shmTimer->seconds;

	if (termTime.ns > 1000000000) {
		termTime.ns -= 1000000000;
		termTime.seconds += 1;
	}
	
	
	while(termTime.seconds > shmTimer->seconds);
	while(termTime.ns > shmTimer->ns);
	
}
/* signal the release the process from the running processes */
sem_wait(semT);
shmT[index] = 1;
sem_post(semT);
printf("USER_PROC %d: Child process is finishing\n", getpid());
sem_post(semD);

errno = shmdt(shmTimer);
if(errno == -1) {
	printf("USER_PROC: shmdt(shmT)\n");
}

errno = shmdt(shmC);
if(errno == -1) {
	printf("USER_PROC: shmdt(shmC)\n");
}

errno = shmdt(shmT);
if(errno == -1) {
	printf("USER_PROC: shmdt(shmT)\n");
}

errno = shmdt(shmP);
if(errno == -1) {
	printf("USER_PROC: shmdt(shmR)\n");
}
//exit(0);
return 0;
}
