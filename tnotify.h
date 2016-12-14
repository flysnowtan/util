#ifndef __TNOTIFY__H__
#define __TNOTIFY__H__

class TNotify
{
public:
	TNotify(int sem_id);
	~TNotify();
	void Post();
	void Wait();
private:
	int m_LockId;
};

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};

#endif 
