#include "updtable.h"
#include "dbformat.h"
#include "leveldb/options.h"

namespace leveldb {

class UpdTable;
class upd_test {

private:

    UpdTable* upd;
    InternalKeyComparator comparator;
    int thres;

public:
    upd_test(int threshold): upd(nullptr), comparator(Options().comparator) {
        thres = threshold;
    }
    ~upd_test(){
        upd->Unref();
    }

    void CreateUpd(){
        upd = new UpdTable(comparator, thres);
        upd->Ref();
    }

    void FillUpd(){
        for(int i = 0; i < thres; i++){
            std::string str = std::to_string(i);
            Slice key(str);
            upd->Add(kMaxSequenceNumber, kTypeValue, key);
        }
    }

    bool IsFull(){
        return upd->isFull();
    }

    void BuildFilter(){
        upd->BuildFilter();
    }

    bool HitBF(int key){
        std::string k = std::to_string(key);
        return upd->Matches(k);
    }

    void Print(){
        upd->printAll();
    }

};

}

int main(){
    leveldb::upd_test test(100);
    test.CreateUpd();
    test.FillUpd();
    assert(test.IsFull());
    test.BuildFilter();

    int k;
    for(int i = 0; i < 10; i++){
        std::scanf("%d", &k);
        printf("%d\n", test.HitBF(k));
    }
    return 0;

}