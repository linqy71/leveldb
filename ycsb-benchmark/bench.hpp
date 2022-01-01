#ifndef _BENCH_HPP
#define _BENCH_HPP


#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <gflags/gflags.h>

typedef enum OP_TYPE {
    OP_UNKNOWN,
    OP_INSERT,
    OP_READ,
    OP_SCAN,
    OP_UPDATE,
    OP_DELETE
}op_type_t;

typedef struct StrOperation{
    op_type_t op;
    std::string key;
    std::string value; // if op == OP_READ, value is the groundtrue result
}str_operation_t;


op_type_t get_op_type_from_string(const std::string & s){
    if (s == "INSERT"){
        return OP_INSERT;
    } else if (s == "READ"){
        return OP_READ;
    } else if (s == "SCAN"){
        return OP_SCAN;
    } else if (s == "UPDATE"){
        return OP_UPDATE;
    } else if (s == "DELETE"){
        return OP_DELETE;
    }
    return OP_UNKNOWN;
}

DEFINE_string(load_file, "NULL" ,"load workload file name");
DEFINE_string(run_file, "NULL" ,"run workload file name");
DEFINE_uint64(ops, 1000 ,"num of ops");

class Benchmark {

private:
	// workload info
	std::string load_workload_file_name;
	std::string run_workload_file_name;

	

public:
	// workload data
	std::vector<double> ps;
	std::vector<str_operation_t> load_ops;
	std::vector<str_operation_t>  run_ops;
	uint64_t num_of_load_ops;
	uint64_t num_of_run_ops;
	// temp info
	std::string step_name;
	std::chrono::system_clock::time_point start_time;
	std::chrono::nanoseconds duration;

	std::vector<int> load_op_counts;
	std::vector<int> run_op_counts;

	Benchmark(int argc, char ** argv) {
		google::ParseCommandLineFlags(&argc, &argv, false);

		std::cout << "load file name : " << FLAGS_load_file << std::endl;
		std::cout << "run file name : " << FLAGS_run_file << std::endl;

		num_of_load_ops = 0; 
		num_of_run_ops = 0; 

		std::string op_string;
    std::cout << "reading workload files ..." << std::endl;

		// load
		std::string load_filename = FLAGS_load_file;
		std::cout << "load_filename : " << load_filename << std::endl;
		load_workload_file_name = load_filename;
		// readfile
		auto file_start_time = std::chrono::system_clock::now();
		std::ifstream load_file_stream(load_filename);
		std::string key;
		while (load_file_stream >> op_string >> key){
				op_type_t op_type = get_op_type_from_string(op_string);
				// std::cout << "[DEBUG] OP : " << op_string << " " << key << std::endl;
				if (op_type == OP_UNKNOWN){
						std::cout << "Warning : Unknown OP : " << op_string << " " << key << std::endl;
						continue;
				}
				load_ops.push_back(str_operation_t{op_type, key, key});
		}
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - file_start_time).count();
		std::cout << "read load file needs: " << duration << " s " << std::endl;

		// run
		std::string run_filename = FLAGS_run_file;
		std::cout << "run_filename : " << run_filename << std::endl;
		run_workload_file_name = run_filename;
		// readfile
		file_start_time = std::chrono::system_clock::now();
		std::ifstream run_file_stream(run_filename);
		while (run_file_stream >> op_string >> key){
			op_type_t op_type = get_op_type_from_string(op_string);
			// std::cout << "[DEBUG] OP : " << op_string << " " << key << std::endl;
			if (op_type == OP_UNKNOWN){
					std::cout << "Warning : Unknown OP : " << op_string << " " << key << std::endl;
					continue;
			}
			// FIXME: update operation should modify the value ?
			if (op_type == OP_UPDATE){
					// maybe it can modify the value once
					run_ops.push_back(str_operation_t{op_type, key, key + "update"});
			} else if (op_type == OP_SCAN){
					// FIXME: NOT IMPLEMENTED
					int num_of_read;
					run_file_stream >> num_of_read; 
					continue;
			} else {
					run_ops.push_back(str_operation_t{op_type, key, key});
			}
		}
		duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - file_start_time).count();
		std::cout << "read run file needs: " << duration << " s " << std::endl;

		num_of_load_ops = load_ops.size();
		std::cout << "num_of_load_ops is " << num_of_load_ops << std::endl;
		num_of_run_ops = run_ops.size();
		std::cout << "num_of_run_ops is " << num_of_run_ops << std::endl;

		load_op_counts.reserve(6);
		run_op_counts.reserve(6);
		for(int i = 0; i < 6; i++) {
			load_op_counts[i] = 0;
			run_op_counts[i] = 0;
		} 

		for (auto op : load_ops){
				load_op_counts[int(op.op)]++;
		}
		for (auto op : run_ops){
				run_op_counts[int(op.op)]++;
		}

		std::cout << "[load] [total ] : " << load_ops.size() << std::endl;
		std::cout << "[load] [INSERT] : " << load_op_counts[OP_INSERT] <<std::endl;
		std::cout << "[load] [READ  ] : " << load_op_counts[OP_READ] <<std::endl;
		std::cout << "[load] [SCAN  ] : " << load_op_counts[OP_SCAN] <<std::endl;
		std::cout << "[load] [UPDATE] : " << load_op_counts[OP_UPDATE] <<std::endl;
		std::cout << "[load] [DELETE] : " << load_op_counts[OP_DELETE] <<std::endl;
		std::cout << "[run ] [total ] : " << run_ops.size() << std::endl;
		std::cout << "[run ] [INSERT] : " << run_op_counts[OP_INSERT] <<std::endl;
		std::cout << "[run ] [READ  ] : " << run_op_counts[OP_READ] <<std::endl;
		std::cout << "[run ] [SCAN  ] : " << run_op_counts[OP_SCAN] <<std::endl;
		std::cout << "[run ] [UPDATE] : " << run_op_counts[OP_UPDATE] <<std::endl;
		std::cout << "[run ] [DELETE] : " << run_op_counts[OP_DELETE] <<std::endl;

		std::cout << "Init load and run operations ok!" << std::endl <<std::endl;

	}



};


#endif