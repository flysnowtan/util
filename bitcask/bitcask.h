#pragma once

#include <hash_map>
#include <string>
#include <vector>

#include "hashtable.h"
#include "store.h"

class TBitCask
{
	public:
		int Select();
		int Update();
		int Insert();
		int Delete();

	private:
		std::hash_map<std::string, std::vector<MemRecord> > m_HashTable;
		TStore m_Store;
};
