#include "record.h"

int TRecord::Record2Buffer(StoreRecord * pRecord, char **ppBuf, uint32_t * pLen)
{
	uint32_t iBuffLen = RECORD_LEN(pRecord->key_sz, pRecord->val_sz);

	char * pBuf = new char[iBuffLen];
	*ppBuf = pBuf;

	*pBuf = pRecord->key_sz;
	pBuf += sizeof(uint8_t);

	memcpy(pBuf, &(pRecord->val_sz), sizeof(uint32_t));
	pBuf += sizeof(uint32_t);

	memcpy(pBuf, pRecord->key, pRecord->key_sz);
	pBuf += pRecord->key_sz;

	memcpy(pBuf, pRecord->value, pRecord->val_sz);
	pBuf += pRecord->val_sz;

	memcpy(pBuf, &(pRecord->crc), sizeof(uint32_t));
	pBuf += sizeof(uint32_t);

	assert(pBuf - *ppBuf == iBuffLen);
	*pLen = iBuffLen;

    return 0;
}

int TRecord::Buffer2Record(char * pBuf, uint32_t len, StoreRecord * pRecord)
{
	char *pHead = pBuf;
	pRecord->key_sz = *pBuf;
	pBuf += sizeof(uint8_t);

	memcpy(&(pRecord->val_sz), pBuf, sizeof(uint32_t));
	pBuf += sizeof(uint32_t);

	memcpy(pRecord->key, pBuf, pRecord->key_sz);
	pBuf += pRecord->key_sz;

	if(pRecord->val_sz > 0)
	{
		pRecord->value = new char[pRecord->val_sz];
		memcpy(pRecord->value, pBuf, pRecord->val_sz);
	}
	else
	{
		pRecord->value = NULL;
	}

	pBuf += pRecord->val_sz;

	memcpy(&pRecord->crc, pBuf, sizeof(uint32_t));
	pBuf += sizeof(uint32_t);

	assert(pBuf - pHead == len);

    return 0;
}

int TRecord::GetRecordLen(char *pBuf, uint32_t len)
{
	return 0;
}

