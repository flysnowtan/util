#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "shmdata.h"

int main(void)
{
	int running = 1;
	void *shm = NULL;
	struct shared_use_st * shared = NULL;

	char buffer[TEXT_SZ+1];

	int shmid;
	shmid = shmget((key_t)1234, sizeof(struct shared_use_st ), 0666 | IPC_CREAT);
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
	printf("Memory attached at %X\n", (int)shm);
	shared = (struct shared_use_st *)shm;

	while(running)
	{
		while(shared->written == 1)
		{
			sleep(1);
			printf("waitting...\n");
		}

		printf("Enter some text: "); 
		fgets(buffer, TEXT_SZ, stdin);
		strncpy(shared->text, buffer, TEXT_SZ);
		shared->written = 1;

		if(strncmp(buffer, "end", 3) == 0)
		{
			running = 0;
		}
	}

	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	sleep(2);
	exit(EXIT_SUCCESS);

}
