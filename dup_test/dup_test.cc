#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>


#include "leveldb/options.h"
#include "leveldb/db.h"


class my_db{
public:

    my_db(){
        options.create_if_missing = true;
	    options.error_if_exists = false;
        options.upd_table_threshold = 1000000;
        leveldb::Status status = leveldb::DB::Open(options, "/home/lqy/db/testdb", &db);
        if (!status.ok()){
            std::cerr << "Open status:" << status.ToString() << std::endl;
        }
    }

    void populate(int start, int end){
        // generate random key
        int range = end - start;
        int vec_size = range;
        int vec_num = 1;

        if (range > 1000000){
            vec_size = 1000000;
            vec_num = range / 1000000;
        }

        std::vector<std::vector<int>> data(vec_num);
        for(int i = 0; i < vec_num; i++){
            std::vector<int> nums(vec_size);
            for(int j = 0; j < vec_size; j++){
                nums[j] = start + vec_size * i + j;
            }
            data[i] = nums;
        }
        
        //shuffle
        for(int i = 0; i < vec_num; i++){
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(data[i].begin(), data[i].end(), std::default_random_engine(seed));
        }

        std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
        for(int i = 0; i < vec_num; i++){
            for(int j = 0; j < data[i].size(); j++){
                std::string str = std::to_string(data[i][j]);
                leveldb::Slice key(str);
                std::string tmp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
                leveldb::Slice value(str+tmp);
                auto start = std::chrono::system_clock::now();
                leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
                std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now() - start);
                total += duration;
                if(!s.ok()){
                    std::cerr << s.ToString() << std::endl;
                }
            }
        }

        std::cout << "put keys: " << start << "---" << end;
        std::cout << "    " << total.count() / (1000 * 1000) << "ms" << std::endl;
    }

private:
    leveldb::DB* db;
    leveldb::Options options;

};

int main(){
    my_db db;
    int start, end;
    std::cin >> start >> end;
    db.populate(start, end);

    return 0;
}