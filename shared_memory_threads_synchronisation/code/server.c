#ifdef __unix__
#include <stdio.h>
#include <stdlib.h>		// exit()
#include <stdint.h>		// int32_t
#include <unistd.h>		// fork()
#include <sys/types.h>	// Shared Memory
#include <sys/ipc.h>	// Shared Memory
#include <sys/shm.h>	// Shared Memory
#include <string.h>
#include <pthread.h>

#include "include.h"

//
// Global Variables
//
int randomLoopMode;
pthread_mutex_t jobMutex;		// Mutex to control the access to shared memory
pthread_cond_t cond;
int count = 0;
struct DataStructure *shmPtr;	// Stores pointer to shared memory 

int slotWorkerCount[10]; 		// Amount of worker threads awake and calculating on a slot.

int slotPercents[10][32];		// stores 32 indiviudal thread percents for each slot.

clock_t startTimes[10];			// Start time for each slot.

pthread_mutex_t slotMutexs[10];

struct JobStructure 
{
	int32_t number;		// Number to perform factorisation on
	int slotNumber;
	int threadNumber;	// Thread number out of 0-31
} jobStructure;

void parent_process(int argc, char const *argv[]);
void child_process(void);
void create_thread(void);
void myWait(void);
void mySignal(void);
int keyboard_ready(void);
int getFreeSlotIndex(int32_t *slots);
void setupSharedMemory(void);
void createJob(uint32_t number, int freeSlot);
uint32_t rightRotateBits(uint32_t inputWord, int numberOfBitsToRotate);
void addToWorkerCount(int slot);
void minusToWorkerCount(int slot);

int main(int argc, char const *argv[])
{
	//
	// Thread variables
	//
	int i, numberOfWorkers, freeSlot;

	//
	// Initalize Mutex
	//
	pthread_mutex_init(&jobMutex, NULL);
	pthread_cond_init (&cond, NULL);

	for (i = 0; i < 10; i++)
		pthread_mutex_init(&slotMutexs[i], NULL);

	//
	// Initalize Shared Memory
	//
	setupSharedMemory();

	// Set num of worker threads from argv[1]
	if (argv[1] != NULL && argv[2] != NULL)
	{
		numberOfWorkers = atoi(argv[1]);
		randomLoopMode = 1;
	}
	else if(argv[1] != NULL)
		numberOfWorkers = atoi(argv[1]);
	else
		numberOfWorkers = DEFAULT_NUM_WORKERS;

	//
	// Create worker threads
	//
	for(i = 0; i < numberOfWorkers; i++)
		create_thread();
	
	//
	// Do server Stuff here
	//
	while(1)
	{
		if (shmPtr->shutdownFlag == 1)
		{
			break;
		}
		if(shmPtr->clientFlag == 1)	// Has client written to number?
		{
			printf("Client Flag: %d\n", shmPtr->clientFlag);
			if((freeSlot = getFreeSlotIndex(shmPtr->slot)) != -1) // Is there a free slot to process number?
			{
				printf("Creating job for slot[%d] number: %d\n", freeSlot, shmPtr->number);
				
				createJob(shmPtr->number, freeSlot);

				shmPtr->number = freeSlot;	// Write index of job slot back to number for client
				shmPtr->clientFlag = 0;		// Set client flag to 0 so client knows to check number
			}
		}
		sleep(1);
	}

	puts("Shutting down..");

	puts("Destroying Mutexs");
	// Destroy mutexs
	pthread_mutex_destroy(&jobMutex);
	for (i = 0; i < 10; i++)
		pthread_mutex_destroy(&slotMutexs[i]);

	puts("Signal finish shutdown");
	// Signal finish of shutdown
	shmPtr->shutdownFlag = 0;

	// Detach from the segment
	if (shmdt(shmPtr) == -1)
	{
		perror("shmdt: ");
		exit(1);
	}

	puts("Destroying threads");
	// All threads are destroyed when this main exits.
	return 0;	
}

void myWait()
{
	pthread_mutex_lock(&jobMutex);
	while(count <= 0)
		pthread_cond_wait(&cond, &jobMutex);
}

void mySignal()
{
	pthread_mutex_lock(&jobMutex);
	count++;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&jobMutex);
}

void *worker_main()
{
	int slot, msec, threadNumber, percentComplete;
	uint32_t number, i;
	float tempPercentComplete;
	clock_t diff;

	puts("Worker Thread Started");
	while(1)
	{
		myWait(); // Blocking semaphore

		number = jobStructure.number;
		slot = jobStructure.slotNumber;
		threadNumber = jobStructure.threadNumber;
		//printf("Thread Number:%d Slot[%d] number: %zu\n",threadNumber, slot, number);
		printf("(%zu) Job Recevied by worker thread. Part %d of 31. Slot[%d]\n", number, threadNumber, slot);
		
		jobStructure.number = 0;

		count--;
		pthread_mutex_unlock(&jobMutex);

		//
		// Calculation of factors
		//
		for(i = 1; i < number; i++)
		{
			if (number % i == 0)
			{
				pthread_mutex_lock(&slotMutexs[slot]);

				//printf("Factor Found: %zu\n", i);
				//
				// Add to shared memory for client to read
				//
				while(shmPtr->serverFlag[slot] != 0)
					sleep(0.5);
				shmPtr->slot[slot] = i;
				shmPtr->serverFlag[slot] = 1;

				pthread_mutex_unlock(&slotMutexs[slot]);
			}
			if(randomLoopMode)
			{
				slotPercents[slot][threadNumber] = (int)((float)i/number * 100);
				percentComplete = calcPercent(slot);
				//printf("PERCENT %d\n", percentComplete);
			}
			else
			{
				percentComplete = (int)((float)i/number * 100);
			}
			shmPtr->progress[slot] = percentComplete;
			//usleep(1); // Makes Calculations really slow
		}
		while(shmPtr->serverFlag[slot] != 0) // Client has not read yet?
			sleep(1);

		slotWorkerCount[slot]--; // Minus one from worker count

		if (slotWorkerCount[slot] < 0) // If last worker on this slot.
		{
			// Last thread to finish
			shmPtr->progress[slot] = 0; 		// Reset progress
			i = 0;								// Just re-using this 32bit int
			shmPtr->slot[slot] = i;				// Reset to 0
			diff = clock() - startTimes[slot];	// Calc time taken in millieseconds.
			msec = diff * 1000 / CLOCKS_PER_SEC;
			printf("\tFinished all for slot[%d]. ", slot);
			printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
			if(diff == 0) // For querys that are less than 1 ms
				diff++;
			shmPtr->finishTimes[slot] = diff;	// Give finish time to client (This also signals that the calculation is complete)
		}
		else // More threads to finish calculations
		{
			printf("Finished %d of 32 for slot[%d]\n", slotWorkerCount[slot], slot);
		}
	}
	return;
}

void create_thread()
{
	pthread_t thread; // this variable is our reference to the second thread

	// Create Worker thread
	if(pthread_create(&thread, NULL, worker_main, NULL)) 
	{
		fprintf(stderr, "Error creating thread\n");
		return;
	}
}

int getFreeSlotIndex(int32_t *slots)
{
	int max = MAX_COMMANDS, i;

	for (i = 0; i < max; i++)
	{
		if (slots[i] == 0 && shmPtr->finishTimes[i] == 0) // If slot is not in use and the finish time slot is not in use
		{
			//printf("Free slot found at position: %d\n", i);
			return i;
		}
	}
	return -1;
}

void setupSharedMemory()
{
	key_t key;
	int shmId;
	void *shmCurrentPtr; // Stores current location in shared memory - Have to cast it to whatever you have stored

	key = KEYNUM; 

	// Create shared memory segment
	if ((shmId = shmget(key, sizeof(dataStructure), 0644 | IPC_CREAT)) == -1)
	{
		perror("shmgeet: ");
		exit(1);
	}

	// Get pointer to shared memory segment
	shmPtr = shmat(shmId, (void*)0, 0); // Paremeter 3 can be SHM_RDONLY for read only access
	if (shmPtr == (struct DataStructure*)(-1))
	{
		perror("shmat: ");
		exit(1);
	}

	puts("Initializing shared memory to nulls");
	memset(shmPtr, '\0', sizeof(dataStructure));
	return;
}

void createJob(uint32_t number, int freeSlot)
{
	int rotations = 32;
	int i;

	if (randomLoopMode == 1)
	{
		// Random Ruben loop
		for (i = 0; i < rotations; i++)
		{
			while(jobStructure.number != 0)
				usleep(1000);
			// Create Job with number
			jobStructure.number = number;
			jobStructure.slotNumber = freeSlot;
			jobStructure.threadNumber = i;

			slotWorkerCount[freeSlot]++; // Add to worker count

			mySignal(); // Wake worker thread

			//sleep(1); // Give other thread some time to access job.

			number = rightRotateBits(number, 1); //Right shift one bit
			//printf("Job created for number: %u\n", number);
			printf("(%zu) Job Created\n", number);
		}	
	}
	else
	{
		// Create Job with number
		jobStructure.number = number;
		jobStructure.slotNumber = freeSlot;

		startTimes[freeSlot] = clock(); // Set Start Time for job

		mySignal(); // Wake worker thread

		//sleep(1); // Give other thread some time to access job.
		printf("(%zu) Job Created\n", number);
		//printf("Job created for number: %zu\n", number);
	}
}

uint32_t rightRotateBits(uint32_t inputWord, int numberOfBitsToRotate) {
	int bitWidth = sizeof(inputWord) * 8;
    // Rotating 32 bits on a 32-bit integer is the same as rotating 0 bits;
    //   33 bits -> 1 bit; etc.
	numberOfBitsToRotate = numberOfBitsToRotate % bitWidth;

	uint32_t tempWord = inputWord;

    // Rotate input to the right
	inputWord = inputWord >> numberOfBitsToRotate;

    // Build mask for carried over bits
	tempWord = tempWord << (bitWidth - numberOfBitsToRotate);

	return inputWord | tempWord;
}

int calcPercent(int slot)
{
	int i, sum = 0;

	for (i = 0; i < 32; i++)
		sum += slotPercents[slot][i];

	return (sum / 32);
}

#endif // END __unix__

#ifdef _WIN32

#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <Strsafe.h>
#include <wchar.h>
#include <winnt.h>
#include <process.h>

 int WorkDone[10];

 DWORD WINAPI WorkThread(PVOID pThreadNum);

int main(int argc , char *argv[])
{
	int k = 10;
    
    HANDLE threads[10];
    

    int i;
	
	puts("hii");
    for(i = 0; i < 10; i++)
    {
    	WorkDone[i] = 0;
    	threads[i] = (HANDLE)_beginthreadex(NULL, 0, WorkThread, (PVOID)&i, 0, NULL);
    }

    WaitForMultipleObjects(10, threads, TRUE, INFINITE);

    for (i = 0; i < 10; i++)
    {
    	printf("Thread %d did %d workunits\n", i, WorkDone[i]);
    }

    return 0;
}

DWORD WINAPI WorkThread(PVOID pThreadNum)
{
	DWORD ThreadNum = (DWORD)&pThreadNum;
	//while(1)
	{
		printf("Thread: %ld\n", ThreadNum);
		WorkDone[ThreadNum]++;
	}
	_endthreadex(0);
}
#endif // END __WIN32