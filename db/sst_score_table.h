#ifndef STORAGE_LEVELDB_DB_SSTSCORETABLE_H_
#define STORAGE_LEVELDB_DB_SSTSCORETABLE_H_

#include <unordered_map>
#include <cassert>

namespace leveldb {

	struct ScoreSst {
		int score;
		uint64_t sst_id;
	};

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

			bool Find(uint64_t sst_id) {
				return score_table.find(sst_id) != score_table.end();
			}

			ScoreSst GetHighScoreSst() {
				uint64_t sst_id = 0;
				int cur_high_score = -1;
				for (auto it: score_table) {
					if (it.second > cur_high_score) {
						cur_high_score = it.second;
						sst_id = it.first;
					}
				}
				ScoreSst result;
				result.score = cur_high_score;
				result.sst_id = sst_id;
				return result;
			}

			~ScoreTable(){}

		private:
			std::unordered_map<uint64_t, int> score_table;

			
	};

}

#endif