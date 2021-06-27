#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>


#include "leveldb/options.h"
#include "leveldb/db.h"


class my_db{
public:

	my_db(){
		options.create_if_missing = true;
		options.error_if_exists = false;
		leveldb::Status status = leveldb::DB::Open(options, "/home/lqy/ether_db/test", &db);
		if (!status.ok()){
				std::cerr << "Open status:" << status.ToString() << std::endl;
		}
	}

	void loadData(std::string path){
			
		std::ifstream data_file;
		data_file.open(path, std::ios::binary);

		std::string line;
		std::string value = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890\
												abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
		std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
		while(getline(data_file, line)){
			auto start = std::chrono::system_clock::now();
			leveldb::Status s = db->Put(leveldb::WriteOptions(), line, value);
			std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
									std::chrono::system_clock::now() - start);
			total += duration;
		}

			std::cout << "load keys from " << path << " done." << std::endl;
			std::cout << "total:" << total.count() / (1000 * 1000) << "ms" << std::endl;
	}

	void readData(std::string path){
		std::ifstream data_file;
		data_file.open(path, std::ios::binary);

		std::string line;
		std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
		while(getline(data_file, line)){
			auto start = std::chrono::system_clock::now();
			std::string value;
			leveldb::Status s = db->Get(leveldb::ReadOptions(), line, &value);
			std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
									std::chrono::system_clock::now() - start);
			total += duration;
		}
		std::cout << "read keys from " << path << " done." << std::endl;
		std::cout << "total:" << total.count() / 1000 << "μs" << std::endl;
	}


private:
	leveldb::DB* db;
	leveldb::Options options;

};

int main(){
	my_db db;
	db.loadData("/home/lqy/test-blockchain/load.dmp");
	db.readData("/home/lqy/test-blockchain/light-load.dmp");

	return 0;
}