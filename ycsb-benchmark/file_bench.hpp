#ifndef _FILE_BENCH_HPP
#define _FILE_BENCH_HPP


#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <gflags/gflags.h>

DEFINE_string(key_file, "NULL" ,"key file name");
DEFINE_uint64(ops, 100000 ,"number of keys");
DEFINE_uint64(dup_ratio, 4 ,"repeat time of keys");

class Benchmark {

private:
	// workload info
	std::string load_workload_file_name;
	std::string run_workload_file_name;


public:
	// workload data
	std::vector<double> ps;
	std::vector<std::string> load_ops;
	uint64_t num_of_load_ops;
    uint64_t dup_ratio;
	// temp info
	std::chrono::system_clock::time_point start_time;
	std::chrono::nanoseconds duration;

	Benchmark(int argc, char ** argv) {
		google::ParseCommandLineFlags(&argc, &argv, false);
        dup_ratio = FLAGS_dup_ratio;

		std::cout << "key file name : " << FLAGS_key_file << std::endl;

		num_of_load_ops = 0;

		std::string op_string;
    	std::cout << "reading workload files ..." << std::endl;

		// load
		std::string load_filename = FLAGS_key_file;
		std::cout << "load_filename : " << load_filename << std::endl;
		load_workload_file_name = load_filename;
		// readfile
		auto file_start_time = std::chrono::system_clock::now();
		std::ifstream load_file_stream(load_filename);
		std::string key;
		while (load_file_stream >> key){
			load_ops.push_back(key);
            num_of_load_ops++;
            if(num_of_load_ops >= FLAGS_ops)
                break;
		}
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - file_start_time).count();
		std::cout << "read load file needs: " << duration << " s " << std::endl;
        num_of_load_ops = load_ops.size();

        load_file_stream.close();
	}



};


#endif