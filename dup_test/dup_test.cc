#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <fstream>


#include "leveldb/options.h"
#include "leveldb/db.h"


class my_timer{
public:
    my_timer(int _max_size){
        max_size = _max_size;
    }

    void insert(std::chrono::nanoseconds& duration){
        if(my_set.size() < max_size){
            my_set.insert(duration);
        } else {
            //compare with smallest latency
            //if duration is smaller, do nothing
            //else, insert duration, remove smallest
            if(duration > *my_set.begin()){
                my_set.erase(my_set.begin());
                my_set.insert(duration);
            }
        }
    }

    std::chrono::nanoseconds get_percentile(double percentile){
        std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
        int number = max_size * (1 - (percentile - 99.0));
        int cnt = 0;
        for(auto it = my_set.rbegin(); it != my_set.rend(); it++){
            total += *it;
            cnt++;
            if(cnt >= number) break;
        }
        return total / number;
    }


private:
    int max_size;
    std::multiset<std::chrono::nanoseconds> my_set;
};

class my_db{
public:

    my_db(std::string path, int upd_thres){
        options.create_if_missing = true;
	    options.error_if_exists = false;
        options.upd_table_threshold = upd_thres;
        leveldb::Status status = leveldb::DB::Open(options, path, &db);
        if (!status.ok()){
            std::cerr << "Open status:" << status.ToString() << std::endl;
        }
    }

    ~my_db(){
        delete db;
    }

    void populateTotallyDup(int start, int end, int dup_ratio, std::string& file_path){
        // generate random key
        int range = end - start;
        int set_max_size = range * 0.01; //  only 1% data saved
        my_timer mt(set_max_size);
        std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();

        auto option = leveldb::WriteOptions();
        // option.sync = true;

        for(int t = 0; t < dup_ratio; t++){
            std::ifstream fin(file_path, std::ios::binary);
            if(!fin) {
                fprintf(stderr, "file %s not exist! \n", file_path.c_str());
                return ;
            }
            std::string line;
            for(int i = 0; i < range; i++){
                getline(fin, line);
                leveldb::Slice key(line.substr(10,64));
                // std::cout << key.ToString() << std::endl;
                auto start = std::chrono::system_clock::now();
                leveldb::Status s = db->Put(option, key, key);
                std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now() - start);
                total += duration;
                mt.insert(duration);
                if(!s.ok()){
                    fprintf(stderr, "put err: %s \n", s.ToString().c_str());
                    return;
                } else {
                    std::string value;
                    leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
                    if(!s.ok()){
                        std::cout << "read key not FOUND! " << std::endl;
                    }
                }
                if(i % 10000 == 0){
                    std::cout << "put keys: " << i << std::endl;
                }
            }
            fin.close();
        }

        std::cout << "put keys: " << start << "---" << end;
        std::cout << "    " << total.count() / (1000 * 1000) << "ms" << std::endl;
        std::cout << "99% latency: " << mt.get_percentile(99.0).count() / 1000 << "us" << std::endl;
        std::cout << "99.9% latency: " << mt.get_percentile(99.9).count() / 1000 << "us" << std::endl;
        std::cout << "99.99% latency: " << mt.get_percentile(99.99).count() / 1000 << "us" << std::endl;
        std::cout << "99.999% latency: " << mt.get_percentile(99.999).count() / 1000 << "us" << std::endl;
    }

    void get(int start, int end, std::string& file_path){
        int range = end - start;
        int set_max_size = range * 0.01; //  only 1% data saved
        my_timer mt(set_max_size);
        std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();

        auto option = leveldb::ReadOptions();

        std::ifstream fin(file_path, std::ios::binary);
        if(!fin) {
            fprintf(stderr, "file %s not exist! \n", file_path.c_str());
            return ;
        }
        std::string line;
        for(int i = 0; i < range; i++){
            getline(fin, line);
            leveldb::Slice key(line.substr(10,64));
            // std::cout << key.ToString() << std::endl;
            std::string value;
            auto start = std::chrono::system_clock::now();
            leveldb::Status s = db->Get(option, key, &value);
            std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now() - start);
            total += duration;
            mt.insert(duration);
            if(!s.ok()){
                fprintf(stderr, "get err: %s \n", s.ToString().c_str());
                return;
            }
            if(i % 100000 == 0){
                std::cout << "get keys: " << i << std::endl;
            }
        }
        fin.close();

        std::cout << "get keys: " << start << "---" << end;
        std::cout << "    " << total.count() / (1000 * 1000) << "ms" << std::endl;
        std::cout << "99% latency: " << mt.get_percentile(99.0).count() / 1000 << "us" << std::endl;
        std::cout << "99.9% latency: " << mt.get_percentile(99.9).count() / 1000 << "us" << std::endl;
        std::cout << "99.99% latency: " << mt.get_percentile(99.99).count() / 1000 << "us" << std::endl;
        std::cout << "99.999% latency: " << mt.get_percentile(99.999).count() / 1000 << "us" << std::endl;

    }

private:
    leveldb::DB* db;
    leveldb::Options options;

};

int main(int argc, char** argv){
    int start, end;
    std::string opt;
    std::string path;
    std::string file_path;
    int dup_ratio;
    int upd_thres;
    for (int i = 1; i < argc; i++){
        char junk;
        std::string str;
        int n;
        if (leveldb::Slice(argv[i]).starts_with("--opt=")) {
            opt = argv[i] + strlen("--opt=");
        } else if (sscanf(argv[i], "--start=%d%c", &n, &junk) == 1){
            start = n;
        } else if (sscanf(argv[i], "--end=%d%c", &n, &junk) == 1){
            end = n;
        }  else if (sscanf(argv[i], "--dup_ratio=%d%c", &n, &junk) == 1){
            dup_ratio = n;
        } else if (leveldb::Slice(argv[i]).starts_with("--db_path=")) {
            path = argv[i] + strlen("--db_path=");
        } else if (sscanf(argv[i], "--upd_thres=%d%c", &n, &junk) == 1) {
            upd_thres = n;
        }  else if (leveldb::Slice(argv[i]).starts_with("--file_path=")) {
            file_path = argv[i] + strlen("--file_path=");
        }
    }
    my_db db(path, upd_thres);
    if(opt == "put") {
        db.populateTotallyDup(start, end, dup_ratio, file_path);
    } else if(opt == "get"){
        db.get(start, end, file_path);
    }

    return 0;
}
