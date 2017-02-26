#include "store.h"

TStore::TStore()
{

}

TStore::~TStore()
{
}

int TStore::Add(StoreRecord& record, int32_t &fileNo, uint32_t &idx)
{
	//lock

	char *pBuf = NULL;
	uint32_t iLen = 0;

	TRecord::Record2Buffer(&record, &pBuf, &iLen);


	//check len
	if(m_CurFile.file_pos + iLen > MAX_FILE_LEN)
	{
		m_CulFile.file_no++;
		m_CulFile.file_pos = 0;
		close(m_Fd);
		m_Fd = -1;
	}

	if(m_Fd == -1)
	{
		m_Fd = Open(m_CulFile.file_no, O_CREAT | O_WRONLY | O_APPEND, 'w');
		if(m_Fd < 0)
		{
			if(pBuf)
			{
				delete[] pBuf;
			}

			return -1;
		}
	}

	//start write

	int iWrite = 0;
	int iLeft = iLen;
	char *pWrite = pBuf;

    while(true)
    {
        iWrite = write(m_Fd, pWrite, iLeft); 
        if(iWrite < 0)
        {
			printf("send err, errno %d", errno);
			if(pBuf)
			{
				delete[] pBuf;
			}
			return -1;
        }
        else
        {
			iLeft -= iWrite;
			pWrite += iWrite;
        }

		if(iLeft == 0)
		{
			break;
		}
    }

	if(pBuf)
	{
		delete[] pBuf;
	}

	fileNo = m_CulFile.file_no;
	idx = m_CulFile.file_pos;
	m_CulFile.file_pos += iLen;
	
	return 0;
}

int TStore::Get(int32_t fileNo, uint32_t idx, StoreRecord& record)
{
	int32_t realFileNo;
	char suffix;

	Transfer2RealFile(fileNo, realFileNo, suffix);
	int fd = Open(realFileNo, O_RDONLY, suffix);	
	if(fd < 0)
	{
		printf("open err\n");
		return -1;
	}






	return 0;
}

int TStore::Open(int32_t fileNo, int iFlag, char suffix)
{
	int fd = -1;
	char fileName[256];

	if(fileNo == 0)
	{
		return -1;
	}

	snprintf(fileName, sizeof(fileName), "%s_%u.%c", m_Path, fileNo, suffix);
	fd = open(fileName, iFlag, 0666);
	if(fd < 0)
	{
		printf("open err!\n");
		return -1;
	}

	return fd;
}

inline void TStore::Transfer2RealFile(int32_t fileNo, int32_t &realFileNo, char& suffix)
{
	readFileNo = fileNo > 0 ? FileNo, -fileNo;
	suffix = fileNo > 0 ? 'w' : 'm';
	return;
}

