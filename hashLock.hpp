#ifndef __HASHLOCK__HPP__
#define __HASHLOCK__HPP__

#include <unistd.h>
#include <fcntl.h>
#include <string>
#include "fileLock.hpp"

class clsHashLock {
	public:
		clsHashLock(const std::string& filename);
		virtual ~clsHashLock();
		int lock(int index);
		int unlock(int index);
		
	private:
		clsFileLock _fileNock;
};

#endif 
