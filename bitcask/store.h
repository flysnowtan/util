#pragma once

#include "fileLock.hpp"

class TStore
{
	public:
		TStore();
		~TStore();
		int Init(char *sPath);
		int Add(StoreRecord& record, int32_t &fileNo, uint32_t &idx);
		int Get(int32_t fileNo, uint32_t idx, StoreRecord& record);
//		int Del(char *key, uint32_t key_size);
	private:
		int Open(int32_t fileNo, int iFlag, char suffix);
		void Transfer2RealFile(int32_t fileNo, int32_t &realFileNo, char& suffix);
	private:
		char *m_Path;
		char * m_FileName;
		uint32_t m_FileSize;
		int m_Fd;
		FilePos m_CurFile;
		std::string m_LockFile;

};
