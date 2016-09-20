#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include "shmdata.h"
#include <string.h>

int main(void)
{
	int running = 1;
	void *shm = NULL;

	struct shared_use_st *shared;
	int shmid;

	shmid = shmget((key_t)1234, sizeof (struct shared_use_st), 0666 | IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	shm = shmat(shmid, 0, 0);

	if(shm == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	printf("\nMemory attached at %x\n", (int) shm);

	shared = (struct shared_use_st *) shm;

	shared->written = 0;
	while(running)
	{
		if(shared->written != 0)
		{
			printf("You worte: %s", shared->text);
			sleep(rand()%3);

			shared->written = 0;

			if(strncmp(shared->text, "end", 3) == 0)
			{
				running = 0;
			}
		}
		else
		{
			sleep(1);
		}
	}

	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	if(shmctl(shmid, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
