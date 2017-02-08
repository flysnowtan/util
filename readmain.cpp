#include "tqueue.h"
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
	TQueue rQueue(SHM_SIZE);

	int ret = rQueue.InitAttach(SHM_KEY, SEM_KEY);
	if(ret != 0)
	{
		printf("ERR! %s->%d InitAttach failed! ret %d\n",
				__func__, __LINE__, ret);
		return 0;
	}

	int i = 0;
	while(true)
	{
		srand(time(NULL));
		TQueueMsg msg;
		ret = rQueue.OutQueue(msg);
		if(ret != 0)
		{
			printf("ERR! %s->%d OutQueue failed! ret %d\n",
					__func__, __LINE__, ret);
			if(ret == ERR_QUEUE_EMPTY)
			{
				continue;
				printf("queue is empty!\n");
			}
			return 0;
		}

		std::string buffer((char*)msg.GetDataBuffer(), msg.GetDataLength());
		printf("out succ! type %d len %d msg %s\n", 
				msg.GetType(), msg.GetLength(), buffer.c_str());
		usleep(rand()%100000);
		i++;
	}
	return 0;
}
