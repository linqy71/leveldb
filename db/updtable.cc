#include "db/updtable.h"
#include "db/dbformat.h"
#include "leveldb/comparator.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"
#include "util/coding.h"


namespace leveldb {

static Slice GetLengthPrefixedSlice(const char* data) {
  uint32_t len;
  const char* p = data;
  p = GetVarint32Ptr(p, p + 5, &len);  // +5: we assume "p" is not corrupted
  return Slice(p, len);
}

UpdTable::UpdTable(const InternalKeyComparator& comparator, const int threshold)
    : comparator_(comparator), refs_(0), table_(comparator_, &arena_),
    policy_(NewBloomFilterPolicy(3)) {
        record_ = 0;
        thres_ = threshold;
    }

UpdTable::~UpdTable() { 
    assert(refs_ == 0);
    delete policy_;
}

void UpdTable::printAll() {
  Iterator* iter = NewIterator();
  iter->SeekToFirst();
  int cnt = 1;
  while (iter->Valid()) {
    printf("%d: %s\n", cnt++, iter->key());
    iter->Next();
  }
  delete iter;
}

size_t UpdTable::ApproximateMemoryUsage() { return arena_.MemoryUsage(); }

int UpdTable::KeyComparator::operator()(const char* aptr,
                                        const char* bptr) const {
  // Internal keys are encoded as length-prefixed strings.
  Slice a = GetLengthPrefixedSlice(aptr);
  Slice b = GetLengthPrefixedSlice(bptr);
  return comparator.Compare(a, b);
}

// Encode a suitable internal key target for "target" and return it.
// Uses *scratch as scratch space, and the returned pointer will point
// into this scratch space.
static const char* EncodeKey(std::string* scratch, const Slice& target) {
  scratch->clear();
  PutVarint32(scratch, target.size());
  scratch->append(target.data(), target.size());
  return scratch->data();
}

class UpdTableIterator : public Iterator {
 public:
  explicit UpdTableIterator(UpdTable::Table* table) : iter_(table) {}

  UpdTableIterator(const UpdTableIterator&) = delete;
  UpdTableIterator& operator=(const UpdTableIterator&) = delete;

  ~UpdTableIterator() override = default;

  bool Valid() const override { return iter_.Valid(); }
  void Seek(const Slice& k) override { iter_.Seek(EncodeKey(&tmp_, k)); }
  void SeekToFirst() override { iter_.SeekToFirst(); }
  void SeekToLast() override { iter_.SeekToLast(); }
  void Next() override { iter_.Next(); }
  void Prev() override { iter_.Prev(); }
  Slice key() const override { return GetLengthPrefixedSlice(iter_.key()); }
  Slice value() const override {
    Slice key_slice = GetLengthPrefixedSlice(iter_.key());
    return GetLengthPrefixedSlice(key_slice.data() + key_slice.size());
  }

  Status status() const override { return Status::OK(); }

 private:
  UpdTable::Table::Iterator iter_;
  std::string tmp_;  // For passing to EncodeKey
};

Iterator* UpdTable::NewIterator() { return new UpdTableIterator(&table_); }

void UpdTable::Add(SequenceNumber s, ValueType type, const Slice& key) {
  // Format of an entry is concatenation of:
  //  key_size     : varint32 of internal_key.size()
  //  key bytes    : char[internal_key.size()]
  size_t key_size = key.size();
  size_t internal_key_size = key_size + 8;
  const size_t encoded_len = VarintLength(internal_key_size) +
                             internal_key_size;
  char* buf = arena_.Allocate(encoded_len);
  char* p = EncodeVarint32(buf, internal_key_size);
  std::memcpy(p, key.data(), key_size);
  p += key_size;
  EncodeFixed64(p, (s << 8) | type);
  assert(p + 8 == buf + encoded_len);
  table_.Insert(buf);
  ++record_;
  if(smallest.size() == 0) {
    smallest = key;
  }
  if(largest.size() == 0){
    largest = key;
  }
  if(comparator_.comparator.Compare(key, smallest) < 0){
    smallest = key;
  } else if(comparator_.comparator.Compare(key, largest) > 0){
    largest = key;
  }
}

void UpdTable::BuildFilter(){ 
    Iterator* iter = NewIterator();
    iter->SeekToFirst();
    std::vector<Slice> keys;
    while(iter->Valid()){
      if(iter->key().size() != 0){
        Slice ukey = ExtractUserKey(iter->key());
        keys.push_back(ukey); //user key
      }
      iter->Next();
    }

    filter_.clear();
    policy_->CreateFilter(&keys[0], static_cast<int>(keys.size()),
                            &filter_);

    delete iter;
}

bool UpdTable::Matches(const Slice& s) {
    if(filter_ == ""){
        //printf("error, filter_ is empty \n");
        return false;
    }
    return policy_->KeyMayMatch(s, filter_);
}

}  // namespace leveldb