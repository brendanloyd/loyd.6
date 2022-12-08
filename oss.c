#include "oss.h"

void sigIntHandler(int signum) {
	printf("OSS: Caught SIGINT! Killing all child processes.");
	
	//clear shared memory
	errno = shmdt(shmTimer);
	if(errno== -1) {
		perror("shmdt(shmTimer) ");	
	}
	
	errno = shmctl(shmidTime, IPC_RMID, NULL);
	if(errno == -1) {
		perror("shmctl(shmidTime) ");	
	}
	
	errno = shmdt(shmC);
	if(errno == -1) {
		perror("shmdt(shmC) ");	
	}
	
	errno = shmctl(shmidC, IPC_RMID, NULL);
	if(errno == -1) {
		perror("shmctl(shmidC ");	
	}	
	
	errno = shmdt(shmT);
	if(errno == -1) {
		perror("shmdt(shTerm ");	
	}
	
	errno = shmctl(shmidT, IPC_RMID, NULL);
	if(errno == -1) {
		perror("shmctl(ShmidT) ");	
	}
	
	errno = shmdt(shmP);
	if(errno == -1) {
		perror("shmdt(shmR) ");	
	}
	
	errno = shmctl(shmidR, IPC_RMID, NULL);
	if(errno == -1) {
		perror("shmctl(shmidR) ");	
	}
	
	// close semaphore, file and exit program
	sem_unlink("semD");   
    	sem_close(semD);
	sem_unlink("semT");
	sem_close(semT);
	sem_unlink("semC");
	sem_close(semC);
	exit(-1);
}

int availableMemory() {
	int i = 0;
	return 0;
}


int main (int argc, char *argv[]) {
FILE *out_file = fopen("./resources.log", "w");
int i, j, k, m, choice;
int children[17] = {0}; 
int maxChildren = 40,maxTime = 5 ,numSlaves = 0 ,numProc = 0;
char buf[200];
signal(SIGINT, sigIntHandler);
time_t start, stop;
struct timer nextProc = {0};
int numShared = 4;
int cycles = 0;
int normTerm = 0;
int pKill = -1; 
float deadTermPercent = 0;
struct message messageBuf;
messageBuf.mtype = 1;
messageBuf.mint = 1000;
messageBuf.pid = getpid();
// random number generator
srand(getpid() * time(NULL));

// option handler
while ((choice = getopt (argc, argv, "h")) != -1)
{
	switch (choice)
	{
		case 'h':
			printf("This program simulates resource management in an operating system using the FIFO method\n");
			exit(1);
			break;
		default:
			break;
	}	
}

//Setup key for message queue
key_t key = ftok("./oss.c", 'B');
        
//Setup id for message queue
int msqid = msgget(key, PERMS | IPC_CREAT);

//Memory allocation
shmidTime = shmget(keyTime, sizeof(struct timer), IPC_CREAT | 0666);
if (shmidTime < 0) {
	perror("shmget(keyTime) ");
	exit(1);
}

shmTimer = shmat(shmidTime, NULL, 0);
if ((void *)shmTimer == (void *)-1) {
	perror("shmat(shmidTime) ");
    	exit(1);
}

shmidC = shmget(keyChild, sizeof(int)*18, IPC_CREAT | 0666);
if (shmidC < 0) {
	perror("shmget(keyChild) ");
	exit(1);
}

shmC = shmat(shmidC, NULL, 0);
if ((void *)shmC == (void *)-1) {
	perror("shmat(shmid(Child) ");
    	exit(1);
}

shmidT = shmget(keyTerm, sizeof(int)*19, IPC_CREAT | 0666);
if (shmidT < 0) {
	perror("shmget(keyTerm) ");
	exit(1);
}

shmT = shmat(shmidT, NULL, 0);
if ((void *)shmT == (void *)-1) {
	perror("shmat(shmidT) ");
  	exit(1);
}

shmidR = shmget(keyRes, sizeof(struct page)*20, IPC_CREAT | 0666);
if (shmidR < 0) {
	perror("shmget(keyRes) ");
	exit(1);
}

shmP = shmat(shmidR, NULL, 0);
if ((void *)shmP == (void *)-1) {
	perror("shmat(shmidR) ");
    	exit(1);
}

//Initilize clock to 0
shmTimer->seconds = 0;
shmTimer->ns = 0;

//set arrays to 0
for(i =0; i<maxChildren; i++) {
	shmC[i] = 0;
	shmT[i] = 0;
}

//Semaphore allocation
semD = sem_open("semD", O_CREAT, 0644, 0);
if(semD == SEM_FAILED) {
	perror("sem_open(semD) ");
    	exit(1);
}

semT = sem_open("semT", O_CREAT, 0644, 1);
if(semT == SEM_FAILED) {
	perror("sem_open(semT) ");
	exit(1);
}    
semC = sem_open("semC", O_CREAT, 0644, 1);
if(semC == SEM_FAILED) {
	perror("sem_open(semC) ");
	exit(1);
}    

//schedule next process to be ran
nextProc.seconds = 0;
nextProc.ns = (rand() % 500000000) + 1;

/* Start the timer */
start = time(NULL);

char timeArgument[10];
char childArgument[10];
char indexArgument[10];
char termArgument[10];
char resArgument[10];

//convert to send to child
sprintf(timeArgument, "%d", shmidTime);
sprintf(childArgument, "%d", shmidC);
sprintf(termArgument, "%d", shmidT);
sprintf(resArgument, "%d", shmidR);
pid_t pid;

pid = fork();
if(pid == 0) {
        pid = getpid();
        shmC[i] = pid;
	execl("./user", "user", timeArgument, childArgument, indexArgument, termArgument, resArgument, (char*)0);
}


msgsnd(msqid, &messageBuf, sizeof(messageBuf), 0);
do {

	//Check if enough time has passed to spawn a child
	/*if(shmTimer->seconds >= nextProc.seconds && shmTimer->ns >= nextProc.ns) {
		
		// Check shmC for first unused process
		sem_wait(semC);
		for(i = 0; i < maxChildren; i++) {
			
			if(shmC[i] == 0) {
				sprintf(indexArgument, "%d", i);
				break;
			}
			
		}
		sem_post(semC);
		
		// Check that number of currently running processes is less than max
		if(numSlaves < maxChildren) {
			nextProc.ns += rand()%500000001;
			if(nextProc.ns >= 1000000000) {
				nextProc.seconds += 1;
				nextProc.ns -= 1000000000;
			}
			numSlaves += 1;
			numProc += 1;
			pid = fork();
			if(pid == 0) {
				pid = getpid();
				shmC[i] = pid;
				execl("./user", "user", timeArgument, childArgument, indexArgument, termArgument, resArgument, (char*)0);
			}
		}
	}*/
	msgrcv(msqid, &messageBuf, sizeof(messageBuf), 1, 0);
	// check memory request and releases	
	if (messageBuf.mint == -1) {
		fprintf(out_file, "Master: recieving termination message: %d from child: %d at time: %d:%d\n", messageBuf.mint, messageBuf.pid, shmTimer->seconds, shmTimer->ns);
	} else if (messageBuf.mint == 0) {
		fprintf(out_file, "Master: recieving write message: %d from child: %d at time: %d:%d\n", messageBuf.mint, messageBuf.pid, shmTimer->seconds, shmTimer->ns);
	} else if (messageBuf.mint == 1) {
                fprintf(out_file, "Master: recieving read message: %d from child %d at time: %d:%d\n", messageBuf.mint, messageBuf.pid, shmTimer->seconds, shmTimer->ns);
        }
	// FIFO algorithm

	messageBuf.pid = getpid();	
	messageBuf.mint = 1000;
	msgsnd(msqid, &messageBuf, sizeof(messageBuf), 0);
	//check for terminating children
	sem_wait(semT);
	for(i = 0; i < maxChildren; i++) {
		
		if(shmT[i] == 1) {
			shmC[i] = 0;
			numSlaves--;
			shmT[i] = 0;
		}
		
	}
	sem_post(semT);
		
	//update clock
	shmTimer->ns += (rand()%10000) + 1;
	if(shmTimer->ns >= 1000000000) {
		shmTimer->ns -= 1000000000;
		shmTimer->seconds += 1;
	}
	cycles++;
	stop = time(NULL);
	
} while(stop-start < maxTime && numProc < 100);

normTerm = numProc;

//kill children
for(i = 0; i < maxChildren; i++) {
	if(shmC[i] != 0) {
		printf("Killing process #%d, PID = %d\n", i, shmC[i]);
		kill(shmC[i], SIGINT);
		sem_wait(semD);
		wait(shmC[i]);
		normTerm--;
	}
}

//Clear memory and close file
shmdt(shmC);
shmctl(shmidC, IPC_RMID, NULL);
shmdt(shmT);
shmctl(shmidT, IPC_RMID, NULL);
shmdt(shmP);
shmctl(shmidR, IPC_RMID, NULL);
sem_unlink("semD");   
sem_close(semD);
sem_unlink("semT");
sem_close(semT);
sem_unlink("semC");
sem_close(semC);
msgctl(msqid, IPC_RMID, NULL);
fclose(out_file);
return 0;
}
