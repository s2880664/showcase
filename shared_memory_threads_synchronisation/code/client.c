#include <stdio.h>
#include <pthread.h>	// Threads
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "include.h"

// Global Variables
struct DataStructure *shmPtr;
uint32_t numberInSlot[10];

void create_thread();
void print_results_main();
void updateProgress();
void clearLine();
void runTestMode();
int getFreeSlotIndex(int32_t *slots);

int main()
{
	//
	// Select values
	//
	fd_set rfds;
    struct timeval tv;
    int retval;
    /* Wait up to five seconds. */
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000; // First number is milliseconds

	shmPtr = getSharedMemoryPointer();

	shmPtr->clientFlag = 0;

	while(1)
	{
		/* Watch stdin (fd 0) to see when it has input. */
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);

	    if (retval == -1)
	        perror("select()");
	    else if (retval)
	    {
	        /* FD_ISSET(0, &rfds) will be true. */

			uint32_t myInt;
			if(scanf("%zu", &myInt) != 1)
			{
				printf("Not an int. Cya\n");
				break;
			}

			if (getFreeSlotIndex(shmPtr->slot) != -1)
			{
				if (myInt == 0)
				{
					runTestMode();
				} 
				else
				{
					while(shmPtr->clientFlag != 0)
						print_results_main();
					clearLine();
					printf("Sending: %zu\n", myInt);
					clearLine();

					shmPtr->number = myInt;
					shmPtr->clientFlag = 1;
					while(shmPtr->clientFlag != 0)
						print_results_main();
					numberInSlot[shmPtr->number] = myInt;
				}
			}
			else
			{
				clearLine();
				puts("Server is busy. Max 10 quries once");
				clearLine();
			}
	    }
	    print_results_main();
	}

	clearLine();

	puts("Setting shutdownFlag to 1");
	shmPtr->shutdownFlag = 1;

	while(shmPtr->shutdownFlag != 0)
		sleep(1);
	puts("Server successfuly shut down");

	puts("Detaching from shared memory");
	// Detach from the segment
	if (shmdt(shmPtr) == -1)
	{
		perror("shmdt: ");
		return 1;
	}
	puts("Successfuly Shutdown");
	return 0;
}

void print_results_main(){
	uint32_t number;
	int i, slot, percentComplete;

	for (i = 0; i < 10; i++) // Slots 0 to 10
	{			
		if(shmPtr->serverFlag[i] == 1) // Factor to be read
		{
			clearLine();
			printf("Slot[%d] Factor = %zu\n", i, shmPtr->slot[i]);
			shmPtr->serverFlag[i] = 0;
		}
		if (shmPtr->finishTimes[i] != 0)
		{
			clearLine();
			printf("\tSlot[%d] (%zu) Completely Finished. Time taken %d seconds %d milliseconds\n", i, numberInSlot[i], shmPtr->finishTimes[i]/1000, shmPtr->finishTimes[i]%1000);
			shmPtr->finishTimes[i] = 0;
		}
	}
	updateProgress();
	return;
}

void updateProgress()
{
	int i, j, percent;

	clearLine();

	for (i = 0; i < MAX_COMMANDS; i++)
	{
		if(shmPtr->slot[i] != 0)
		{
			percent = (shmPtr->progress[i])/10;
			printf("Q%d |", i);
			for (j = 0; j <= 8; j++)
			{
				if (j == 4 || j == 5)
				{
					if (j == 4)
						printf("%d%%", shmPtr->progress[i]);
				}
				else
				{
					if (j < percent)
					{
						printf("▓");
					} 
					else 
					{
						printf("_");
					}
				}
			}
			printf("|   ");
		}
	}
	fflush(stdout);
}

void clearLine()
{
	printf("%c[2K", 27); //Clear the line
	printf("\r");
	return;
}

void runTestMode()
{
	int i, j, current;
	for (i = 0; i < 3 ; i++)
	{
		for (j = 0; j <= 9; ++j)
		{
			current = (i * 10) + j;

			while(shmPtr->clientFlag != 0)
			{
				print_results_main();
				updateProgress();	
			}
			clearLine();
			printf("Sending: %d\n", current);

			shmPtr->number = current;
			shmPtr->clientFlag = 1;

			while(shmPtr->clientFlag != 0)
				print_results_main();
			numberInSlot[shmPtr->number] = current;

			updateProgress();
		}
	}
	clearLine();
	sleep(1);
	puts("Finished Test Mode 0-29");
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