#ifndef STORAGE_LEVELDB_DB_UPDTABLE_H_
#define STORAGE_LEVELDB_DB_UPDTABLE_H_

#include <string>

#include "db/dbformat.h"
#include "db/skiplist.h"
#include "leveldb/db.h"
#include "leveldb/filter_policy.h"
#include "util/arena.h"


namespace leveldb {

class InternalKeyComparator;
class UpdTableIterator;

class UpdTable {
 public:
  // UpdTables are reference counted.  The initial reference count
  // is zero and the caller must call Ref() at least once.
  explicit UpdTable(const InternalKeyComparator& comparator, const int threshold);

  UpdTable(const UpdTable&) = delete;
  UpdTable& operator=(const UpdTable&) = delete;

  // Increase reference count.
  void Ref() { ++refs_; }

  // Drop reference count.  Delete if no more references exist.
  void Unref() {
    --refs_;
    assert(refs_ >= 0);
    if (refs_ <= 0) {
      delete this;
    }
  }

  // Returns an estimate of the number of bytes of data in use by this
  // data structure. It is safe to call when UpdTable is being modified.
  size_t ApproximateMemoryUsage();

  // Return an iterator that yields the contents of the updtable.
  //
  // The caller must ensure that the underlying UpdTable remains live
  // while the returned iterator is live.  The keys returned by this
  // iterator are internal keys encoded by AppendInternalKey in the
  // db/format.{h,cc} module.
  Iterator* NewIterator();

  // Add an entry into updtable that maps key to value at the
  // specified sequence number and with the specified type.
  // Typically value will be empty if type==kTypeDeletion.
  void Add(SequenceNumber seq, ValueType type, const Slice& key);

  bool isFull(){
    return record_ >= thres_;
  }

  void BuildFilter();

 private:
  friend class UpdTableIterator;
  //friend class MemTableBackwardIterator;

  struct KeyComparator {
    const InternalKeyComparator comparator;
    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
    int operator()(const char* a, const char* b) const;
  };

  typedef SkipList<const char*, KeyComparator> Table;

  ~UpdTable();  // Private since only Unref() should be used to delete it

  KeyComparator comparator_;
  int refs_;
  Arena arena_;
  Table table_;

  int record_;
  int thres_;

  const FilterPolicy* policy_;
  std::string filter_;
};

}  // namespace leveldb


#endif // STORAGE_LEVELDB_DB_UPDTABLE_H_