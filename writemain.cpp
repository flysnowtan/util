#include "tqueue.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	TQueue rQueue(SHM_SIZE);
	int ret = rQueue.InitForce(SHM_KEY, SEM_KEY);
	if(ret != 0)
	{
		printf("ERR! %s->%d InitForce failed ret %d\n",
				__func__, __LINE__, ret);
		return 0;
	}
	for(int i = 0; i < 10000; i++)
	{
		char buffer[1024];
		sprintf(buffer, "test%d", i);
		printf("buffer size %d\n", strlen(buffer));
		TQueueMsg msg(i, buffer, strlen(buffer));
		ret = rQueue.InQueue(msg);
		if(ret != 0)
		{
			printf("ERR! InQueue failed! %s->%d ret %d\n",
					__func__, __LINE__, ret);
			if(ret == ERR_QUEUE_FULL)
			{
				sleep(1000);
			}
			return 0;
		}
		usleep(100000);
	}
	return 0;
}
