#ifndef STORAGE_LEVELDB_INCLUDE_MY_L0ITERATOR_H_
#define STORAGE_LEVELDB_INCLUDE_MY_L0ITERATOR_H_

#include "leveldb/iterator.h"
#include "db/table_cache.h"

namespace leveldb {
	class MyL0Iterator : public Iterator {
		private:
			Iterator* table_cache_iter_;
			uint64_t file_num_;

			mutable char key_buf[100];
		
		public:
			MyL0Iterator(Iterator* t_c_iter, uint64_t file_num)
				: table_cache_iter_(t_c_iter), file_num_(file_num) {

			}

			~MyL0Iterator() {
				delete table_cache_iter_;
			}

			bool Valid() const override {
				return table_cache_iter_->Valid();
			}

			void SeekToFirst() override {
				table_cache_iter_->SeekToFirst();
			}

			void SeekToLast() override {
				table_cache_iter_->SeekToLast();
			}

			void Seek(const Slice& target) override {
				table_cache_iter_->Seek(target);
			}

			void Next() override {
				table_cache_iter_->Next();
			}

			void Prev() override {
				table_cache_iter_->Prev();
			}

			Slice key() const override {
				Slice key = table_cache_iter_->key();
				memccpy(key_buf, key.data(), 0, key.size());
				EncodeFixed64(key_buf + key.size(), file_num_);
				return Slice(key_buf, key.size() + 8);
			}

			Slice value() const override {
				return table_cache_iter_->value();
			}

			Status status() const override {
				return table_cache_iter_->status();
			}

	};
}


#endif