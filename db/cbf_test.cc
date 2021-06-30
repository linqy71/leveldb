#include "leveldb/slice.h"
#include "util/counting_bloom.h"
#include <iostream>

namespace leveldb{

class CountingBloomFilter;

class CBFTest{

 private:
  CountingBloomFilter* cbf;
  std::string filter;

 public:
  CBFTest(){
    cbf = new CountingBloomFilter(4);
  }

  void InitFilter(){
    cbf->InitCBF();
  }

  void Add(const Slice& key){
    cbf->AddKey(key);
  }

  int Count(const Slice& key){
    return cbf->KeyCounter(key);
  }

};

}

int main(){
  leveldb::CBFTest test;
  test.InitFilter();
  for(int i = 1000; i < 5000; i++){  //cold
    leveldb::Slice k = std::to_string(i);
    test.Add(k);
  }
  int opt;
  std::string k;
  while(std::cin >> opt){
    std::cin >> k;
    leveldb::Slice key(k);
    if(opt == 1){
      test.Add(key);
    } else {
      std::cout << test.Count(key) << std::endl;
    }
  }

  return 0;
}