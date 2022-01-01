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


#include "bench.hpp"

#include "leveldb/db.h"

using namespace leveldb;


// An example to test the performance of map
void test_str_perf(Benchmark& bm){
	const std::vector<str_operation_t>& load_ops = bm.load_ops;
	const std::vector<str_operation_t>& run_ops = bm.run_ops;
	std::vector<decltype(std::chrono::system_clock::now())> points;
	points.reserve(bm.num_of_load_ops + 1);

	// parameters["str_key_size"] = FLAGS_str_key_size;
	// parameters["str_value_size"] = FLAGS_str_value_size;

	// std::string pool_file_name = "/mnt/hdd/db";
	// std::remove(pool_file_name.c_str());

	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, "/mnt/hdd/db", &db);
	assert(status.ok());

	//start load
	int op_count = bm.num_of_load_ops;
	points[0] = std::chrono::system_clock::now();
	for (int j = 0; j < op_count; j++){
		auto & op = load_ops[j];
		switch (op.op){
			case OP_INSERT:{
				Slice key(op.key.c_str(), op.key.size());
				Slice value(op.value.c_str(), op.value.size());
				db->Put(leveldb::WriteOptions(), key, value);
				break;
			}
			default: {
				std::cout << "Unknown OP In Load : " << op.op << std::endl;
				break;
			}
		}
		points[j + 1] = std::chrono::system_clock::now();
	}

	std::vector<uint64_t> load_latencys(bm.num_of_load_ops,0);
	int index = 0;
	for (int j = 0; j < bm.num_of_load_ops; j++){
		load_latencys[index] = std::chrono::duration_cast<std::chrono::nanoseconds>(points[j+1] - points[j]).count();
		index++;
	}
	std::cout << index << std::endl;

	std::sort(load_latencys.begin(), load_latencys.end());

	std::vector<double> ps = {0.001, 0.005, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.99, 0.999, 0.9999, 0.99999, 0.999999};

	std::cout << "min : " << *(load_latencys.begin()) << std::endl;
	for (int i = 1; i <= 10; i++){
			std::cout << "max : " << *(load_latencys.end()-i) << std::endl;
	}
	for (auto p : ps){
			std::cout << "p "  << p << " : " << load_latencys[(uint64_t)(load_latencys.size()*p)] << std::endl;
	}


	//start run
	bm.start_time = std::chrono::system_clock::now();
	op_count = bm.num_of_run_ops;
	std::string getvalue;
	std::vector<uint64_t> latency_read(bm.run_op_counts[OP_READ], 0);
	std::vector<uint64_t> latency_update(bm.run_op_counts[OP_UPDATE], 0);
	int read_cnt = 0;
	int up_cnt = 0;
	for (int j = 0; j < op_count; j++){
		auto & op = run_ops[j];
		auto start = std::chrono::system_clock::now();
		switch (op.op){
			case OP_READ:{
				Slice key(op.key.c_str(), op.key.size());
				db->Get(leveldb::ReadOptions(), key, &getvalue);
				latency_read[read_cnt++] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start).count();
				break;
			}
			case OP_UPDATE:{
				Slice key(op.key.c_str(), op.key.size());
				Slice value(op.value.c_str(), op.value.size());
				db->Put(leveldb::WriteOptions(), key, value);
				latency_update[up_cnt++] = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start).count();
				break;
			}
			default: {
				std::cout << "Unknown OP In run : " << op.op << std::endl;
				break;
			}
		}
	}

	// assert(read_cnt == bm.run_op_counts[OP_READ]);
	// run latencys

	std::sort(latency_read.begin(), latency_read.end());

	std::sort(latency_update.begin(), latency_update.end());

	// std::vector<double> ps = {0.001, 0.005, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.99, 0.999, 0.9999, 0.99999, 0.999999};

	std::cout << std::endl;
	std::cout << read_cnt << "  " << up_cnt << std::endl;

	std::cout << "read latency : " << std::endl;
	std::cout << "min : " << *(latency_read.begin()) << std::endl;
	for (int i = 1; i <= 10; i++){
			std::cout << "max : " << *(latency_read.end()-i) << std::endl;
	}
	for (auto p : ps){
			std::cout << "p "  << p << " : " << latency_read[(uint64_t)(latency_read.size()*p)] << std::endl;
	}

	std::cout << "update latency : " << std::endl;
	std::cout << "min : " << *(latency_update.begin()) << std::endl;
	for (int i = 1; i <= 10; i++){
			std::cout << "max : " << *(latency_update.end()-i) << std::endl;
	}
	for (auto p : ps){
			std::cout << "p "  << p << " : " << latency_update[(uint64_t)(latency_update.size()*p)] << std::endl;
	}


	return ;
}

int main(int argc, char ** argv){
    Benchmark bm(argc, argv);

    test_str_perf(bm);
    
    return 0;
}