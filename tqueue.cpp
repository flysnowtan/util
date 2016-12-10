#include "tqueue.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

TQueueMsg::TQueueMsg()
{
	m_pMsgHeader = NULL;
}

TQueueMsg::TQueueMsg(int type, char *data, int len)
{
	m_pMsgHeader = (TMsgHeader*) malloc(len + sizeof(TMsgHeader));
	m_pMsgHeader->m_MsgSize = len + sizeof(TMsgHeader);
	m_pMsgHeader->m_Type = type;
	memcpy((char*)m_pMsgHeader+sizeof(TMsgHeader), data, len);
}

TQueueMsg::~TQueueMsg()
{
	if(m_pMsgHeader)
	{
		free((void*)m_pMsgHeader);
	}
}

void TQueueMsg::SetBuffer(char *data, int len)
{
	if(m_pMsgHeader)
	{
		free(m_pMsgHeader);
	}
	m_pMsgHeader = (TMsgHeader*)malloc(len);
	memcpy((char*)m_pMsgHeader, data, len);
}

int TQueueMsg::GetLength()
{
	return m_pMsgHeader->m_MsgSize;
}

int TQueueMsg::GetDataLength()
{
	return m_pMsgHeader->m_MsgSize - sizeof(TMsgHeader);
}

void* TQueueMsg::GetBuffer()
{
	return m_pMsgHeader;
}

int TQueueMsg::GetType()
{
	return m_pMsgHeader->m_Type;
}

void* TQueueMsg::GetDataBuffer()
{
	return m_pMsgHeader->m_Msg;
}

TQueue::TQueue(uint32_t iQueueSize)
{
	if(iQueueSize < sizeof(QueueHeader))
	{
		printf("queue size is too smail!\n");
	}
	m_Size = iQueueSize;
	m_LockId = -1;
	m_pQueue = NULL;
}

TQueue::~TQueue()
{
	if(m_pQueue)
	{
		if(-1 == shmdt(m_pQueue))
		{
			printf("ERR! %s shmdt failed!\n", __func__);
		}
	}
}

int TQueue::InitAttach(int shm_key, int lock_key)
{
	int shm_id = shmget((key_t)shm_key, 0, 0);
	if(shm_id == -1)
	{
		printf("ERR! %s shm is not exist\n", __func__);
		return -1;
	}
	
	m_pQueue = (QueueHeader*)shmat(shm_id, NULL, SHM_R|SHM_W);
	if((void*)m_pQueue == (void*)-1)
	{
		printf("ERR! %s->%d shmat failed! errno %d\n",
				__func__, __LINE__, errno);
		return -1;
	}
	return 0;
}

int TQueue::InitForce(int shm_key, int lock_key)
{
	int shm_id = shmget((key_t)shm_key, 0, 0666);
	if(shm_id != -1)
	{
		shmctl(shm_id, IPC_RMID, NULL);
	}
	shm_id = shmget((key_t)shm_key, m_Size+sizeof(QueueHeader), 0666 | IPC_CREAT | IPC_EXCL);
	if(shm_id == -1)
	{
		printf("ERR! %s->%d shmget failed! errno %d\n",
				__func__, __LINE__, errno);
		return -1;
	}

	m_pQueue = (QueueHeader*)shmat(shm_id, NULL, 0);
	if((void*)m_pQueue == (void*)-1)
	{
		printf("ERR! %s->%d shmat failed! errno %d\n",
				__func__, __LINE__, errno);
		return -1;
	}

	m_pQueue->m_Read = 0;
	m_pQueue->m_Write = 0;
	m_pQueue->m_Count = 0;

	return 0;
}

int TQueue::InQueue(TQueueMsg &msg)
{
	printf("write %d read %d count %d\n", m_pQueue->m_Write, m_pQueue->m_Read, m_pQueue->m_Count);
	int iMsgLen = msg.GetLength();
	if(iMsgLen < 0 || iMsgLen > m_Size)
	{
		printf("ERR! %s msg is too big or small!\n",
				__func__);
		return -1;
	}
	int iDistToEnd =  m_Size - m_pQueue->m_Write;
	if(m_pQueue->m_Write < m_pQueue->m_Read)
	{
		if(m_pQueue->m_Read-m_pQueue->m_Write < iMsgLen)
		{
			printf("ERR! %s queue is full, can not inqueue\n",
					__func__);
			return ERR_QUEUE_FULL;
		}
	}
	else if(m_pQueue->m_Write > m_pQueue->m_Read)
	{
		int iCanWriteLen = m_Size - m_pQueue->m_Write + m_pQueue->m_Read;
		if(iCanWriteLen < iMsgLen)
		{
			printf("ERR! %s queue is not enough to write the msg!\n",
					__func__);
			return ERR_QUEUE_FULL;
		}
	}
	else if(m_pQueue->m_Count > 0)
	{
		printf("ERR! write=read %s queue is full!\n",
				__func__);
		return ERR_QUEUE_FULL;
	}

	if(iDistToEnd >= iMsgLen)
	{
		memcpy(&m_pQueue->m_Msg[m_pQueue->m_Write], msg.GetBuffer(), iMsgLen);
		m_pQueue->m_Write += iMsgLen;
		if(m_pQueue->m_Write == m_Size)
		{
			m_pQueue->m_Write = 0;
		}
	}
	else
	{
		memcpy(&m_pQueue->m_Msg[m_pQueue->m_Write], msg.GetBuffer(), iDistToEnd);
		memcpy(&m_pQueue->m_Msg[0], (char*)msg.GetBuffer()+iDistToEnd, iMsgLen-iDistToEnd);
		m_pQueue->m_Write = iMsgLen-iDistToEnd;
	}

	m_pQueue->m_Count++;
	return QUEUE_OPER_OK;
}

int TQueue::OutQueue(TQueueMsg &msg)
{
	printf("write %d read %d count %d\n", m_pQueue->m_Write, m_pQueue->m_Read, m_pQueue->m_Count);
	if(m_pQueue->m_Count <= 0)
	{
		printf("ERR! %s queue is empty\n", __func__);
		return ERR_QUEUE_EMPTY;
	}
	int iDistToEnd = m_Size-m_pQueue->m_Read;
	int iDist = m_pQueue->m_Write - m_pQueue->m_Read;
	if((iDist > 0 && iDist < sizeof(int))
			|| (iDist < 0 && m_Size + iDist < sizeof(int)))
	{
		printf("ERR! %s it's not a complete msg!\n",
				__func__);
		return -1;
	}

	int iMsgLen = 0;
	if(iDist > 0)
	{
		memcpy(&iMsgLen, &m_pQueue->m_Msg[m_pQueue->m_Read], sizeof(int));
	}
	else
	{
		if(iDistToEnd >= sizeof(int))
		{
			memcpy(&iMsgLen, &m_pQueue->m_Msg[m_pQueue->m_Read], sizeof(int));
		}
		else
		{
			char tmpLen[8];
			memcpy(tmpLen, &m_pQueue->m_Msg[m_pQueue->m_Read], iDistToEnd);
			memcpy(tmpLen+iDistToEnd, &m_pQueue->m_Msg[0], sizeof(int) - iDistToEnd);
			memcpy(&iMsgLen, tmpLen, sizeof(int));
		}
	}
	printf("get msg len %d\n", iMsgLen);

	if(iMsgLen < 0 
			|| iMsgLen > m_Size
			|| (iDist > 0 && iDist < iMsgLen)
			|| (iDist < 0 && m_Size + iDist < iMsgLen))
	{
		printf("ERR! %s wrong msg\n", __func__);
		return -1;
	}

	if(iDist > 0 
			|| (iDist <= 0 && iDistToEnd >= iMsgLen))
	{
		msg.SetBuffer(&m_pQueue->m_Msg[m_pQueue->m_Read], iMsgLen);
		m_pQueue->m_Read += iMsgLen;
		if(m_pQueue->m_Read == m_Size)
		{
			m_pQueue->m_Read = 0;
		}
	}
	else 
	{
		char *buffer = (char*)malloc(iMsgLen);
		memcpy(buffer, &m_pQueue->m_Msg[m_pQueue->m_Read], iDistToEnd);
		memcpy(buffer+iDistToEnd, &m_pQueue->m_Msg[0], iMsgLen-iDistToEnd);
		msg.SetBuffer(buffer, iMsgLen);
		m_pQueue->m_Read = iMsgLen-iDistToEnd;
	}

	m_pQueue->m_Count--;
	return 0;
}
