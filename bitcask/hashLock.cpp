#include "hashLock.hpp"

clsHashLock::clsHashLock(const std::string& filename)
	:_fileNock(filename)
{
}

clsHashLock::~clsHashLock()
{
}

int clsHashLock::lock(int index)
{
	return _fileNock.lockW(index, 1);	
}

int clsHashLock::unlock(int index)
{
	return _fileNock.unlock(index, 1);
}
