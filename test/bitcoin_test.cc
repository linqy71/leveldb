#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>


#include "leveldb/options.h"
#include "leveldb/db.h"


class my_db{
public:

	my_db(){
		options.create_if_missing = true;
		options.error_if_exists = false;
		leveldb::Status status = leveldb::DB::Open(options, "/home/lqy/bitcoin_db/test", &db);
		if (!status.ok()){
				std::cerr << "Open status:" << status.ToString() << std::endl;
		}
	}

	void loadData(std::string path){
			
		std::ifstream data_file;
		data_file.open(path, std::ios::binary);

		std::string line;
		// std::string value = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890\
		// 										abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
		std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
        long cnt = 0;
		while(getline(data_file, line)){
			auto start = std::chrono::system_clock::now();
            std::string key = line.substr(10,64);
			leveldb::Status s = db->Put(leveldb::WriteOptions(), key, line);
			std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
									std::chrono::system_clock::now() - start);
			total += duration;
            if(++cnt % 100000 == 0) {
                printf("put %d keys done.\n", cnt);
            }
		}
        std::cout << "load keys from " << path << " done." << std::endl;
        std::cout << "total:" << total.count() / (1000 * 1000) << "ms" << std::endl;
        std::cout << "average:" << total.count() / (1000 * cnt) << "μs/op" << std::endl;
	}

	void readData(std::string path){
		std::ifstream data_file;
		data_file.open(path, std::ios::binary);

		std::string line;
		std::chrono::nanoseconds total = std::chrono::nanoseconds::zero();
        long cnt = 0;
		while(getline(data_file, line)){
			auto start = std::chrono::system_clock::now();
            std::string key = line.substr(10,64);
			std::string value;
			leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
			std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
									std::chrono::system_clock::now() - start);
			total += duration;
            if(++cnt % 100000 == 0) {
                printf("get %d keys done.\n", cnt);
            }
		}
		std::cout << "read keys from " << path << " done." << std::endl;
		std::cout << "total:" << total.count() / (1000 * 1000) << "ms" << std::endl;
        std::cout << "average:" << total.count() / (1000 * cnt) << "μs/op" << std::endl;
	}


private:
	leveldb::DB* db;
	leveldb::Options options;

};

int main(){
	my_db db;
	db.loadData("/home/lqy/bitcoin-source/bitcoin/transactions.json");
	db.readData("/home/lqy/bitcoin-source/bitcoin/transactions.json");

	return 0;
}