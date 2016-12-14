#include "tnotify.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>

TNotify::TNotify(int lock_key)
{
	m_LockId = semget((key_t)lock_key, 1, 0666 | IPC_CREAT);
	if(m_LockId == -1)
	{
		printf("ERR! %s->%d semget failed! errno %d\n",
				__func__, __LINE__, errno);
	}

	union semun sem_ctr;
	sem_ctr.val = 1;
	if(semctl(m_LockId, 0, SETVAL, sem_ctr) == -1)
	{
		printf("ERR! %s->%d semctl failed! errno %d\n",
				__func__, __LINE__, errno);
	}
}

TNotify::~TNotify()
{

}

void TNotify::Post()
{
	struct sembuf sem_buf;
	sem_buf.sem_num = 0;
	sem_buf.sem_op = 1;
	sem_buf.sem_flg = 0;
	if(semop(m_LockId, &sem_buf, 1) == -1)
	{
		printf("ERR! %s->%d semop failed! errno %d", 
				__func__, __LINE__, errno);
	}
}

void TNotify::Wait()
{
	struct sembuf sem_buf;
	sem_buf.sem_num = 0;
	sem_buf.sem_op = -1;
	sem_buf.sem_flg = 0;
	if(semop(m_LockId, &sem_buf, 1) == -1)
	{
		printf("ERR! %s->%d semop failed! errno %d", 
				__func__, __LINE__, errno);
	}
}
