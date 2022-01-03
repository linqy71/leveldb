#include <thread>
#include <cstdio>
#include <vector>
// #include <cinttypes>
#include <inttypes.h>
#include <string>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <chrono>

#include "file_bench.hpp"

#include "leveldb/db.h"

using namespace leveldb;


// An example to test the performance of map
void test_str_perf(Benchmark& bm){
	const std::vector<std::string>& load_ops = bm.load_ops;

	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, "/mnt/hdd/db", &db);
	assert(status.ok());

	// insert and update
    int op_count_total = bm.num_of_load_ops;
    int op_count = 1000000;
    int epoch = op_count_total / op_count;
    std::vector<uint64_t> write_durations(op_count_total, 0);
    uint64_t n = bm.dup_ratio;
    std::vector<uint64_t> update_durations(bm.num_of_load_ops * n, 0);
	
    std::chrono::system_clock::time_point start;
    for (int e = 0; e < epoch; e++) {
        int start_index = e * op_count;

        //insert
        for (int j = start_index; j < op_count + start_index; j++){
            auto & key_str = load_ops[j];
            Slice key(key_str.c_str(), key_str.size());
            Slice value(key_str.c_str(), key_str.size());
            start = std::chrono::system_clock::now();
            db->Put(leveldb::WriteOptions(), key, value);
            write_durations[j] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start).count();
        }
        //update
        
        for (int i = 0; i < n; i++) { //repeat update
            for (int j = start_index; j < op_count + start_index; j++){
                auto & key_str = load_ops[j];
                Slice key(key_str.c_str(), key_str.size());
                Slice value(key_str.c_str(), key_str.size());
                start = std::chrono::system_clock::now();
                db->Put(leveldb::WriteOptions(), key, value);
                update_durations[op_count * i + j] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start).count();
            }
        }
    }

    
	std::sort(write_durations.begin(), write_durations.end());

	std::vector<double> ps = {0.001, 0.005, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.99, 0.999, 0.9999, 0.99999, 0.999999};

    std::cout << "insert latency: " << std::endl;
	std::cout << "min : " << *(write_durations.begin())<< std::endl;
	for (int i = 1; i <= 10; i++){
			std::cout << "max : " << *(write_durations.end()-i) << std::endl;
	}
	for (auto p : ps){
			std::cout << "p "  << p << " : " << write_durations[(uint64_t)(write_durations.size()*p)] << std::endl;
	}

    // update
    
	std::sort(update_durations.begin(), update_durations.end());
    std::cout << std::endl;
    std::cout << "update latency: " << std::endl;
	std::cout << "min : " << *(update_durations.begin()) << std::endl;
	for (int i = 1; i <= 10; i++){
			std::cout << "max : " << *(update_durations.end()-i) << std::endl;
	}
	for (auto p : ps){
			std::cout << "p "  << p << " : " << update_durations[(uint64_t)(update_durations.size()*p)] << std::endl;
	}

    // read
    std::vector<uint64_t> read_durations(bm.num_of_load_ops, 0);
    std::string getValue;
    for (int j = 0; j < op_count; j++){
        auto & key_str = load_ops[j];
        Slice key(key_str.c_str(), key_str.size());
        start = std::chrono::system_clock::now();
        db->Get(leveldb::ReadOptions(), key, &getValue);
        read_durations[j] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start).count();
    }
	std::sort(read_durations.begin(), read_durations.end());
    std::cout << std::endl;
    std::cout << "read latency: " << std::endl;
	std::cout << "min : " << *(read_durations.begin()) << std::endl;
	for (int i = 1; i <= 10; i++){
			std::cout << "max : " << *(read_durations.end()-i) << std::endl;
	}
	for (auto p : ps){
			std::cout << "p "  << p << " : " << read_durations[(uint64_t)(read_durations.size()*p)] << std::endl;
	}

	return ;
}

int main(int argc, char ** argv){
    Benchmark bm(argc, argv);

    test_str_perf(bm);
    
    return 0;
}