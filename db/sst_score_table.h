#ifndef STORAGE_LEVELDB_DB_SSTSCORETABLE_H_
#define STORAGE_LEVELDB_DB_SSTSCORETABLE_H_

#include <unordered_map>
#include <cassert>

namespace leveldb {

	class ScoreTable {
		public:

			explicit ScoreTable(){}

			void AddScore(uint64_t sst_id) {
				auto it = score_table.find(sst_id);
				if(it != score_table.end()) {
					score_table[sst_id] = it->second + 1;
				} else {
					score_table[sst_id] = 1;
				}
			}

			void RemoveSstScore(uint64_t sst_id) {
				score_table.erase(sst_id);
			}

			uint64_t GetHighScoreSst() {
				uint64_t result = 0;
				int cur_high_score = -1;
				for (auto it: score_table) {
					if (it.second > cur_high_score) {
						cur_high_score = it.second;
						result = it.first;
					}
				}
				return result;
			}

			~ScoreTable(){}

		private:
			std::unordered_map<uint64_t, int> score_table;

			
	};

}

#endif