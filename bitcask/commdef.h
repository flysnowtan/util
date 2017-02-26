#pragma once

#define MAX_KEY_LEN 8
#define MAX_FILE_LEN 1024*1024*2

struct StoreRecord
{
	uint8_t key_sz;
	uint32_t val_sz;
    char  key[MAX_KEY_LEN]; 
    char * value;
    uint32_t crc;
};

struct FilePos
{
	int32_t file_no;
	uint32_t file_pos;
};



