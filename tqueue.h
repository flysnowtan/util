
#include "tqueueglobaldef.h"
#include <stdint.h>

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
	QueueHeader *m_pQueue;
	uint32_t m_Size;
	int m_LockId;
};


