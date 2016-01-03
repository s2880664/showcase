#include <sys/types.h>	// Shared Memory
#include <sys/ipc.h>	// Shared Memory
#include <sys/shm.h>	// Shared Memory
#include <stdlib.h>		// exit()
#include <time.h>		// Timing

#define KEYNUM 9874	// Set to a random number - May be unsafe.
#define MAX_COMMANDS 10
#define DEFAULT_NUM_WORKERS 32

//
// Global Structure
//
struct DataStructure 
{
	uint32_t number;
	uint32_t slot[10];
	char progress[10];
	char clientFlag;
	char serverFlag[10];
	clock_t finishTimes[10];
	int shutdownFlag; // boolean
} dataStructure;

//
// Global Functions
//
struct DataStructure * getSharedMemoryPointer()
{
	//
	// Shared memory stuff
	//
	key_t key;
	int shmId;
	struct DataStructure *shmPtr; 			// Stores start of shared memory - Have to cast it to whatever you have stored
	void *shmCurrentPtr;	// Stores current location in shared memory - Have to cast it to whatever you have stored

	key = KEYNUM; 

	// Create shared memory segment
	if ((shmId = shmget(key, sizeof(dataStructure), 0644 | IPC_CREAT)) == -1)
	{
		perror("shmget: ");
		exit(1);
	}

	// Get pointer to shared memory segment
	shmPtr = shmat(shmId, (void*)0, 0); // Paremeter 3 can be SHM_RDONLY for read only access
	if (shmPtr == (struct DataStructure*)(-1))
	{
		perror("shmat: ");
		exit(1);
	}
	return shmPtr;
}