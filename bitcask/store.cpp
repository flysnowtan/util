#include "store.h"

TStore::TStore()
{

}

TStore::~TStore()
{

}

int TStore::Get(char *key, uint32_t key_size, char* &value, uint32_t &value_size)
{
}

int TStore::Set(char *key, uint32_t key_size, char *value, uint32_t value_size)
{
	StoreRecord record;
	record.key_size = key_size;
	record.value_size = value_size
	record.key = key;
	record.value = value;

}

int TStore::Del(char *key, uint32_t key_size)
{

}

