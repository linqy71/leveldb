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
        leveldb::Status status = leveldb::DB::Open(options, "/home/lqy/db/testdb_dup", &db);
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
                std::string tmp1 = "abcdefghijklmno1234567890";
                leveldb::Slice key(str + tmp1);
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

    void get(int start, int end){
        int range = end - start;
        std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
        std::vector<std::chrono::nanoseconds> times(range);
        std::string tmp1 = "1234567890";
        for(int i = 0; i < range; i++){
            int k = start + i;
            leveldb::Slice key(std::to_string(k) + tmp1);
            std::string value;
            auto start = std::chrono::system_clock::now();
            db->Get(leveldb::ReadOptions(), key, &value);
            std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now() - start);
            times[i] = duration;
            total += duration;
        }

        std::sort(times.begin(), times.end());
        int p99 = range * 0.99;
        std::chrono::nanoseconds p99_sum = std::chrono::nanoseconds::zero();
        for(int i = p99; i < range; i++){
            p99_sum += times[i];
        }
        int p01 = range * 0.01;
        std::chrono::nanoseconds p99_avg = p99_sum / p01;
        std::cout << "get keys " << start << "---" << end;
        std::cout << ". total: " << total.count() / (1000 * 1000) << "ms";
        std::cout << "; p99: " << p99_avg.count() / (1000) << "us" << std::endl;


    }

private:
    leveldb::DB* db;
    leveldb::Options options;

};

int main(){
    my_db db;
    int start, end;
    std::string opt;
    std::cin >> opt;
    
    std::cin >> start >> end;
    if(opt == "put") {
        db.populate(start, end);
    } else if(opt == "get"){
        db.get(start, end);
    }
    

    return 0;
}