#ifndef STORAGE_LEVELDB_DB_GLOBALTABLE_H_
#define STORAGE_LEVELDB_DB_GLOBALTABLE_H_

#include <vector>
#include <unordered_map>

#include "dbformat.h"

namespace leveldb {

class InternalKey;

struct Entry{
	InternalKey small;
	InternalKey large;
};


class GlobalTable{

public:

	GlobalTable(){
        table_ = new std::unordered_map<uint64_t, int>();
    }

	void AddEntry(uint64_t file_num, int level){
		table_->emplace(file_num, level);
	}

	int GetLevel(uint64_t target_filenum){
		auto it = table_->find(target_filenum);
		if(it != table_->end()) return it->second;
		return -1;
	}

	void RemoveFile(uint64_t target_filenum){
		auto it = table_->find(target_filenum);
		if(it != table_->end()) {
			table_->erase(it);
		}
	}

	std::unordered_map<uint64_t, int>* GetTable(){
		return table_;
	}

	~GlobalTable(){
		delete table_;
	}


private: 
	// map filenum to level
	std::unordered_map<uint64_t, int>* table_;

};

}


#endif