#pragma once

struct StoreRecord
{
	uint32_t key_size;
	uint32_t value_size;
	char * key;
	char * value;
};

class TStore
{
	public:
		TStore();
		~TStore();
		int Get(char *key, uint32_t key_size, char* &value, uint32_t &value_size);
		int Set(char *key, uint32_t key_size, char *value, uint32_t value_size);
		int Del(char *key, uint32_t key_size);
	private:
		char * m_FileName;
		uint32_t m_FileSize;
		int m_Fd;
};
