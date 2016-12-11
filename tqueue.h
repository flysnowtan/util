
#include "tqueueglobaldef.h"
#include <stdint.h>


union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};

struct TMsgHeader
{
	int m_MsgSize;
	int m_Type;
	char m_Msg[0];
};

class TQueueMsg
{	
public:
	TQueueMsg();
	TQueueMsg(int type, char *data, int len);
	~TQueueMsg();
	int GetType();
	int GetLength();
	int GetDataLength();
	void* GetBuffer();
	void *GetDataBuffer();
	void SetBuffer(char *data, int len);
private:
	TMsgHeader *m_pMsgHeader;
};

struct QueueHeader
{
	int m_Read;
	int m_Write;
	int m_Count;
	char m_Msg[0];
};

//读写进程必须通过配置文件约定共享内存的大小，而且要一致。
class TQueue
{
public:
	TQueue(uint32_t iQueueSize = 1024*1024);
	~TQueue();
	int InitForce(int shm_key, int lock_key);
	int InitAttach(int shm_key, int lock_key);
	int InQueue(TQueueMsg &msg);
	int OutQueue(TQueueMsg &msg);
private:
	void Lock();
	void UnLock();
	QueueHeader *m_pQueue;
	uint32_t m_Size;
	int m_LockId;
};


