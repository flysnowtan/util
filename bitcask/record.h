#pragma once

#include "commdef.h"

#define RECORD_LEN(key_sz, val_sz) (sizeof(uint8_t) + sizeof(uint32_t) + key_sz + val_sz + sizeof(uint32_t))
class TRecord
{
    public:
    static int Record2Buffer(StoreRecord * pRecord, char **ppBuf, uint32_t * pLen);
    static int Buffer2Record(char * pBuf, uint32_t len, StoreRecord * pRecord);
};
