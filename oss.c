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
	
	errno = shmdt(shmR);
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
	fclose(out_file);
	exit(-1);
}

int deadlock(unsigned int maxRes, unsigned int children, unsigned int numShared) {
	unsigned int work[maxRes]; 
	unsigned int finish[children]; 
	int deadlocked = 0, numLocked = 0, i, j;
	for(i = 0; i < maxRes; i++) {
		work[i] = shmR[i].available;
	}
	for(i = 0; i < children; i++) {
		finish[i] = 0;
	}
	for(j = 0; j < children; j++) {
		if(finish[j]) {
			continue;
		}
		if(avail(work, j, maxRes, numShared)) {
			finish[j] = 1;
			for(i = 0; i < maxRes; i++)
			work[i] += shmR[i].allArray[j];
			j = -1;
		}
	}
	for(j = 0; j < children; j++) {
		if(!finish[j]) {			
			numLocked++;
			lockProc[j] = 1;
			if(numLocked >= 2)
			{
				deadlocked = 1;
			}
		}
	}
	return(deadlocked);
}

int avail(unsigned int *work, unsigned int procNum, unsigned int maxRes, unsigned int numShared) {
	int i;
	for(i = 0; i < maxRes; i++) {
		if(shmR[i].reqArray[procNum] > work[i]) {
			break;
		}
	}
	return(i == maxRes);
}

void deadClear(int maxRes, int children, int numShared) {
	int i, j;
	int killed = -1;
	int lastKilled = shmT[19];
	for(i = 0; i < children; i++) {
		if(lockProc[i] == 1 && killed == -1) {
			if(logCount < 100000) {
				fprintf(out_file, "\tKilling child process P%d:\n", i);
				logCount += 1;
			}
			if(logCount < 100000) {
				fprintf(out_file, "\tResources released are: ");
				for(j = 0; j < maxRes; j++) {
					if(shmR[j].allArray[i] > 0) {
						fprintf(out_file, "R%d:%d, ", j, shmR[j].allArray[i]);
					}
				}
				fprintf(out_file, "\n");
				logCount += 1;
			}
			kill(shmC[i], SIGINT);
			sem_wait(semD);
			wait(shmC[i]);
			killed = i;
		}
		if(killed != -1) {
			lockProc[i] = 0;
		}
	}
	if(killed != -1) {
		if(logCount < 100000) {
			fprintf(out_file, "\tOSS running deadlock avoidance after P%d killed\n", killed);
			logCount += 1;
		}
		if(deadlock(maxRes, children, numShared)) {
			if(logCount < 100000) {
				fprintf(out_file, "\tProcesses ");
				for(i = 0; i < children; i++) {
					if(lockProc[i] == 1) {
						fprintf(out_file, "P%2d, ", i);
					}
				}
				fprintf(out_file, "deadlocked\n");
				logCount += 1;
			}
			deadClear(maxRes, children, numShared);
		}
	}
}

int main (int argc, char *argv[]) {
char *fileName = "./resources.log";
int i, j, k, m, choice;
int children[17] = {0}; 
int maxChildren = 40,maxTime = 5 ,numSlaves = 0 ,numProc = 0, verbose = 0;
char buf[200];
signal(SIGINT, sigIntHandler);
time_t start, stop;
struct timer nextProc = {0};
int numShared = 4;
int cycles = 0;
int pKill = -1; 
int deadlocked = 0, deadCount = 0, deadAvoid = 0;
int requests = 0, normTerm = 0;
int DARuns = 0;
float deadTermPercent = 0;

// random number generator
srand(getpid() * time(NULL));

// option handler
while ((choice = getopt (argc, argv, "hv")) != -1)
{
	switch (choice)
	{
		case 'h':
			printf("This program simulates resource management in an operating system using deadlock avoidance\n");
			printf("-v\tVerbose option: this option affects what information is saved in the log file.\nUsage:\t ./oss -v\n");
			exit(1);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			break;
	}	
}

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

shmidR = shmget(keyRes, sizeof(struct resource)*20, IPC_CREAT | 0666);
if (shmidR < 0) {
	perror("shmget(keyRes) ");
	exit(1);
}

shmR = shmat(shmidR, NULL, 0);
if ((void *)shmR == (void *)-1) {
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

//allocate resources
for(i = 0; i < 20; i++) {
	shmR[i].maxAmt = (rand() % 10) + 1;
	shmR[i].available = shmR[i].maxAmt;
}

//set shared resources
for(i = 0; i < numShared; i++) {
	shmR[i].shared = 1;
}
for(i = numShared; i < 20; i++) {
	shmR[i].shared = 0;
}

// Allocation for resource arrays in shmR
for(i = 0; i < 20; i++) {
	for(j = 0; j < maxChildren; j++) {
		shmR[i].reqArray[j] = 0;
		shmR[i].allArray[j] = 0;
		shmR[i].relArray[j] = 0;
	}
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
do {

	//Check if enough time has passed to spawn a child
	if(shmTimer->seconds >= nextProc.seconds && shmTimer->ns >= nextProc.ns) {
		
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
	}
	
	//check for SHARED resource requests and releases
	for(i = 0; i < numShared; i++) {
		for(j = 0; j < maxChildren; j++) {
			if(shmR[i].reqArray[j] == 1 && shmR[i].allArray[j] < shmR[i].maxAmt) {
				if(logCount < 100000 && verbose == 1) {
					fprintf(out_file, "OSS granting P%d request R%d at time %d:%d\n", j, i, shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
				requests++;

				if(requests % 20 == 0) {
					
					//display tables
					if(logCount < 100000 && verbose == 1) {
						fprintf(out_file,"Current System Resources:\n");
						logCount += 1;
					}

					if(logCount < 100000 && verbose == 1) {
						fprintf(out_file,"Resource:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19\n");
						logCount += 1;
					}

					if(logCount < 100000 && verbose == 1) {
						fprintf(out_file,"Shared:    ");
						for(m = 0; m < 20; m++) {
							fprintf(out_file, "%2d ", shmR[m].shared);
						}
						fprintf(out_file, "\n");
						logCount += 1;
					}

					if(logCount + maxChildren < 100000 && verbose == 1) {
						for(k = 0; k < maxChildren; k++) {
							fprintf(out_file,"P%2d:       ", k);
							for(m = 0; m < 20; m++) {
								fprintf(out_file, "%2d ", shmR[m].allArray[k]);
							}
							fprintf(out_file,"\n");
							logCount += 1;
						}
					}
				}
				shmR[i].reqArray[j] = 0;
				shmR[i].allArray[j]++;
			}
			if(shmR[i].relArray[j] >= 1) {
		
				if(logCount < 100000 && verbose == 1) {
					fprintf(out_file, "OSS acknowledged Process P%d releasing R%d at time %d:%d\n", j, i, shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
				shmR[i].allArray[j] -= shmR[i].relArray[j];
				shmR[i].relArray[j] = 0;
			}
		}
	}
	
	//check resource requests and releases
	for(i = numShared; i < 20; i++) {

		for(j = 0; j < maxChildren; j++) {

			if(shmR[i].reqArray[j] == 1 && shmR[i].allocation < shmR[i].maxAmt) {

				if(logCount < 100000 && verbose == 1) {
					fprintf(out_file, "OSS granting P%d request R%d at time %d:%d\n", j, i, shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
				requests++;

				if(requests % 20 == 0) {

					//display tables
					if(logCount < 100000 && verbose == 1) {
						fprintf(out_file,"Current System Resources:\n");
						logCount += 1;
					}
					if(logCount < 100000 && verbose == 1) {
						fprintf(out_file,"Resource:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19\n");
						logCount += 1;
					}
					if(logCount < 100000 && verbose == 1) {
						fprintf(out_file,"Shared:    ");
						for(m = 0; m < 20; m++) {
							fprintf(out_file, "%2d ", shmR[m].shared);
						}
						fprintf(out_file,"\n");
						logCount += 1;
					}
					if(logCount + maxChildren < 100000 && verbose == 1) {
						for(k = 0; k < maxChildren; k++) {
							fprintf(out_file,"P%2d:       ", k);
							for(m = 0; m < 20; m++) {
								fprintf(out_file, "%2d ", shmR[m].allArray[k]);
							}
							fprintf(out_file,"\n");
							logCount += 1;
						}
					}
				}
				shmR[i].allocation++;
				shmR[i].available--;
				shmR[i].reqArray[j] = 0;
				shmR[i].allArray[j]++;
			}
			if(shmR[i].relArray[j] >= 1) {
				if(logCount < 100000 && verbose == 1) {
					fprintf(out_file, "OSS acknowledged Process P%d releasing R%d at time %d:%d\n", j, i, shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
				shmR[i].allocation -= shmR[i].relArray[j];
				shmR[i].available += shmR[i].relArray[j];
				shmR[i].allArray[j] -= shmR[i].relArray[j];
				shmR[i].relArray[j] = 0;
			}
		}
	}
	
	// Deadlock algorithm
	if(cycles%DDA == 0 && numSlaves == maxChildren) {
			if(logCount < 100000) {
				fprintf(out_file, "OSS running deadlock avoidance at time %d:%d\n", shmTimer->seconds, shmTimer->ns);
				logCount += 1;
			}
			DARuns++;
			if(deadlock(20, maxChildren, numShared)) {
				deadlocked = 1;
				deadCount++;
				if(logCount < 100000) {
					fprintf(out_file, "\tProcesses ");
					for(i = 0; i < maxChildren; i++) {
						if(lockProc[i] == 1) {
							fprintf(out_file, "P%2d, ", i);
							totLocked++;
						}
					}
					fprintf(out_file, "unsafe zone\n");
					logCount += 1;
				} else {
					for(i = 0; i < maxChildren; i++) {
						if(lockProc[i] == 1) {
							totLocked++;
						}
					}
				}
				if(logCount < 100000) {
					fprintf(out_file,"\tAvoiding deadlock.. \n", shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
				deadClear(20, maxChildren, numShared);
				if(logCount < 100000) {
					fprintf(out_file, "\tSystem is no longer in deadlock\n", shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
			} else {
				if(logCount < 100000) {
					fprintf(out_file, "System is not in unsafe zone\n", shmTimer->seconds, shmTimer->ns);
					logCount += 1;
				}
				deadlocked = 0;
			}
	}
	
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
	deadAvoid = shmT[19];
	cycles++;
	stop = time(NULL);
	
} while(stop-start < maxTime && numProc < 100);

deadAvoid = shmT[19];
normTerm = numProc - deadAvoid;

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
//print results
printf("Requests Granted: %d\n", requests);
printf("Number of aborted Processes: %d\n", deadAvoid);
printf("Number of Normally Terminated Processes: %d\n", normTerm);
printf("Number of DA Runs: %d\n", DARuns);

//Clear memory and close file
shmdt(shmTimer);
shmctl(shmidT, IPC_RMID, NULL);
shmdt(shmC);
shmctl(shmidC, IPC_RMID, NULL);
shmdt(shmT);
shmctl(shmidT, IPC_RMID, NULL);
shmdt(shmR);
shmctl(shmidR, IPC_RMID, NULL);
sem_unlink("semD");   
sem_close(semD);
sem_unlink("semT");
sem_close(semT);
sem_unlink("semC");
sem_close(semC);
fclose(out_file);
return 0;
}
