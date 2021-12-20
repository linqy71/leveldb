#ifndef STORAGE_LEVELDB_DB_KEYUPDLRU_H_
#define STORAGE_LEVELDB_DB_KEYUPDLRU_H_

#include <string>

#include "db/sst_score_table.h"
#include "leveldb/cache.h"
#include "leveldb/slice.h"


namespace leveldb {

	// value -> void*
	static void* EncodeValue(uintptr_t v) { return reinterpret_cast<void*>(v); }

	// void* -> value and delete
	static void DeleteValue(const Slice& key, void* value) {
		uint64_t* v = reinterpret_cast<uint64_t*>(value);
		delete v;
	}


	class KeyUpdLru {
	private:
		Cache* sstid_cache_;
	public:
		KeyUpdLru(const int kCacheSize) : sstid_cache_(NewLRUCache(kCacheSize)) {

		}
		~KeyUpdLru() {
			delete sstid_cache_;
		}

		// add <key, SST_id> pair to updmap
		// key is userkey
		// if key already exists, the old SST_id score+1
		void Add(Slice& key, uint64_t SST_id, ScoreTable* score_tbl) {

			Cache::Handle* handle = sstid_cache_->Lookup(key);
			if (handle != nullptr) { //find key, so add score
				uint64_t* id_ptr = reinterpret_cast<uint64_t*>(sstid_cache_->Value(handle));
				sstid_cache_->Release(handle);
				//old id score +1
				score_tbl->AddScore(*id_ptr);
			}
			
			sstid_cache_->Insert(key, EncodeValue(SST_id), 1, DeleteValue);
		}

		bool FindSst(Slice& key, uint64_t* value) {

			Cache::Handle* handle = sstid_cache_->Lookup(key);
			if (handle != nullptr) { //find key
				uint64_t* id_ptr = reinterpret_cast<uint64_t*>(sstid_cache_->Value(handle));
				value = id_ptr;
				sstid_cache_->Release(handle);
				return true;
			}

			return false;
		}

	};
	
}


#endif