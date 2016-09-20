#ifndef __SHMDATA__H__
#define __SHMDATA__H__

#define TEXT_SZ 2048

struct shared_use_st
{
	int written;
	char text[TEXT_SZ];
};


#endif
