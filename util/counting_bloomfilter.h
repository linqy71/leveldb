#include "leveldb/filter_policy.h"

#include "leveldb/slice.h"
#include "util/hash.h"

namespace leveldb{

static uint32_t BloomHash(const Slice& key) {
  return Hash(key.data(), key.size(), 0xbc9f1d34);
}

class CountingBloomFilter {
 public:
  explicit CountingBloomFilter(int cnter_per_key) : cnter_per_key_(cnter_per_key){
    //暂时不考虑像bloom那样用0.69缩减空间
    capacity_ = 100000; //at most 10^7 keys;
  }

  void InitCBF() {
    std::string* dst = &filter_;
    size_t bits = capacity_ * cnter_per_key_ * 4; // 4 bit per counter
    size_t bytes = (bits + 7) / 8;
    bits = bytes * 8;

    const size_t init_size = dst->size();
    dst->resize(init_size + bytes, 0);
  }

  void AddKey(const Slice& key){
    std::string* dst = &filter_;
    size_t bits = capacity_ * cnter_per_key_ * 4; // 4 bit per counter
    size_t bytes = (bits + 7) / 8;
    bits = bytes * 8;
    uint32_t h = BloomHash(key);
    char* array = &(*dst)[0];
    const uint32_t delta = (h >> 17) | (h << 15); 
    for(size_t j = 0; j < cnter_per_key_; j++){
      const uint32_t bit_pos = h % bits;
      const uint32_t cnter_pos = bit_pos / 4;
      const uint32_t byte_pos = bit_pos / 8;
      if (cnter_pos % 2 == 0) { //low 4 bits
        uint8_t cnt = (int)(array[byte_pos] & 0xF);// read counter
        cnt = (cnt + 1) > 15? 15: cnt + 1; // convert to int and add
        array[byte_pos] &= (0xF << 4); // save counter to array
        array[byte_pos] |= cnt;
      } else { //high 4 bits
        uint8_t cnt = (int)(array[byte_pos] & 0xF0) >> 4;
        cnt = (cnt + 1) > 15? 15: cnt + 1;
        array[byte_pos] &= 0xF; // save counter to array
        array[byte_pos] |= cnt << 4;
      }

      h += delta;
    }

    //reduce 1 counter randomly
    srand((int)time(0));
    uint32_t rand_pos = rand() % bits;
    uint32_t rand_cnter_pos = rand_pos / 4;
    uint32_t rand_byte_pos = rand_pos / 8;
    uint8_t cnt = rand_cnter_pos % 2 == 0? (int)(array[rand_byte_pos] & 0xF): 
                  (int)(array[rand_byte_pos] & 0xF0) >> 4;
    if(cnt > 0){
      cnt--;
      if(rand_cnter_pos % 2 == 0){
        array[rand_byte_pos] &= (0xF << 4); // save counter to array
        array[rand_byte_pos] |= cnt;
      } else {
        array[rand_byte_pos] &= 0xF; // save counter to array
        array[rand_byte_pos] |= cnt << 4;
      }
    }
  }

  int KeyCounter(const Slice& key){
    const size_t len = filter_.size();
    if (len < 2) return 0;

    const char* array = filter_.data();
    const size_t bits = len * 8;

    uint32_t h = BloomHash(key);
    const uint32_t delta = (h >> 17) | (h << 15);  // Rotate right 17 bits
    
    int smallest = 16;
    for(size_t j = 0; j < cnter_per_key_; j++){
      const uint32_t bit_pos = h % bits;
      const uint32_t cnter_pos = bit_pos / 4;
      const uint32_t byte_pos = bit_pos / 8;
      uint8_t cnt = 0;
      if (cnter_pos % 2 == 0) { //low 4 bits
        cnt = (int)(array[byte_pos] & 0xF);// read counter
      } else {
        cnt = (int)(array[byte_pos] & 0xF0) >> 4;
      }
      if (cnt < smallest){
        smallest = cnt;
        if(smallest == 0) break;
      }
      h += delta;
    }
    return smallest;
  }



 private:
  int cnter_per_key_;
  std::string filter_;
  int capacity_;
};

} // namespace leveldb