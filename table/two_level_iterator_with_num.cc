
#include "table/two_level_iterator_with_num.h"

#include "leveldb/table.h"
#include "table/block.h"
#include "table/format.h"
#include "table/iterator_wrapper.h"
#include "util/coding.h"

namespace leveldb {

namespace {

typedef Iterator* (*BlockFunction)(void*, const ReadOptions&, const Slice&);

class TwoLevelIteratorWithNum : public Iterator {
 public:
  TwoLevelIteratorWithNum(Iterator* index_iter, BlockFunction block_function,
                   void* arg, const ReadOptions& options);

  ~TwoLevelIteratorWithNum() override;

  void Seek(const Slice& target) override;
  void SeekToFirst() override;
  void SeekToLast() override;
  void Next() override;
  void Prev() override;

  bool Valid() const override { return data_iter_.Valid(); }
  // returns key with file_num in last 8 bytes
  Slice key() const override {
    assert(Valid());
    // data key + file_num
    Slice data_key = data_iter_.key();
    size_t data_size = data_key.size();
    memccpy(key_buf_, data_key.data(), 0, data_size);
    Slice index = index_iter_.value();
    uint64_t file_num = DecodeFixed64(index.data());
    EncodeFixed64(key_buf_ + data_size, file_num);
    return Slice(key_buf_, data_size + 8);
  }
  Slice value() const override {
    assert(Valid());
    return data_iter_.value();
  }
  Status status() const override {
    // It'd be nice if status() returned a const Status& instead of a Status
    if (!index_iter_.status().ok()) {
      return index_iter_.status();
    } else if (data_iter_.iter() != nullptr && !data_iter_.status().ok()) {
      return data_iter_.status();
    } else {
      return status_;
    }
  }

 private:
  void SaveError(const Status& s) {
    if (status_.ok() && !s.ok()) status_ = s;
  }
  void SkipEmptyDataBlocksForward();
  void SkipEmptyDataBlocksBackward();
  void SetDataIterator(Iterator* data_iter);
  void InitDataBlock();

  BlockFunction block_function_;
  void* arg_;
  const ReadOptions options_;
  Status status_;
  IteratorWrapper index_iter_;
  IteratorWrapper data_iter_;  // May be nullptr
  // If data_iter_ is non-null, then "data_block_handle_" holds the
  // "index_value" passed to block_function_ to create the data_iter_.
  std::string data_block_handle_;
  mutable char key_buf_[100];
};

TwoLevelIteratorWithNum::TwoLevelIteratorWithNum(Iterator* index_iter,
                                   BlockFunction block_function, void* arg,
                                   const ReadOptions& options)
    : block_function_(block_function),
      arg_(arg),
      options_(options),
      index_iter_(index_iter),
      data_iter_(nullptr) {}

TwoLevelIteratorWithNum::~TwoLevelIteratorWithNum() = default;

void TwoLevelIteratorWithNum::Seek(const Slice& target) {
  index_iter_.Seek(target);
  InitDataBlock();
  if (data_iter_.iter() != nullptr) data_iter_.Seek(target);
  SkipEmptyDataBlocksForward();
}

void TwoLevelIteratorWithNum::SeekToFirst() {
  index_iter_.SeekToFirst();
  InitDataBlock();
  if (data_iter_.iter() != nullptr) data_iter_.SeekToFirst();
  SkipEmptyDataBlocksForward();
}

void TwoLevelIteratorWithNum::SeekToLast() {
  index_iter_.SeekToLast();
  InitDataBlock();
  if (data_iter_.iter() != nullptr) data_iter_.SeekToLast();
  SkipEmptyDataBlocksBackward();
}

void TwoLevelIteratorWithNum::Next() {
  assert(Valid());
  data_iter_.Next();
  SkipEmptyDataBlocksForward();
}

void TwoLevelIteratorWithNum::Prev() {
  assert(Valid());
  data_iter_.Prev();
  SkipEmptyDataBlocksBackward();
}

void TwoLevelIteratorWithNum::SkipEmptyDataBlocksForward() {
  while (data_iter_.iter() == nullptr || !data_iter_.Valid()) {
    // Move to next block
    if (!index_iter_.Valid()) {
      SetDataIterator(nullptr);
      return;
    }
    index_iter_.Next();
    InitDataBlock();
    if (data_iter_.iter() != nullptr) data_iter_.SeekToFirst();
  }
}

void TwoLevelIteratorWithNum::SkipEmptyDataBlocksBackward() {
  while (data_iter_.iter() == nullptr || !data_iter_.Valid()) {
    // Move to next block
    if (!index_iter_.Valid()) {
      SetDataIterator(nullptr);
      return;
    }
    index_iter_.Prev();
    InitDataBlock();
    if (data_iter_.iter() != nullptr) data_iter_.SeekToLast();
  }
}

void TwoLevelIteratorWithNum::SetDataIterator(Iterator* data_iter) {
  if (data_iter_.iter() != nullptr) SaveError(data_iter_.status());
  data_iter_.Set(data_iter);
}

void TwoLevelIteratorWithNum::InitDataBlock() {
  if (!index_iter_.Valid()) {
    SetDataIterator(nullptr);
  } else {
    Slice handle = index_iter_.value();
    if (data_iter_.iter() != nullptr &&
        handle.compare(data_block_handle_) == 0) {
      // data_iter_ is already constructed with this iterator, so
      // no need to change anything
    } else {
      Iterator* iter = (*block_function_)(arg_, options_, handle);
      data_block_handle_.assign(handle.data(), handle.size());
      SetDataIterator(iter);
    }
  }
}

}  // namespace

Iterator* NewTwoLevelIteratorWithNum(Iterator* index_iter,
                              BlockFunction block_function, void* arg,
                              const ReadOptions& options) {
  return new TwoLevelIteratorWithNum(index_iter, block_function, arg, options);
}

}  // namespace leveldb
