//
// Created by daiyi on 2019/09/30.
//

#ifndef LEVELDB_STATS_H
#define LEVELDB_STATS_H


#include <cstdint>
#include <map>
#include <vector>
#include <cstring>
#include "timer.h"

using std::string;
using std::to_string;


namespace MyStat {

    class Timer;

    typedef enum TASK_TYPE {
        TASK_FLUSH,
        TASK_GET,
        TASK_CBF,
        TASK_COMPACTION,
        TASK_WRITE,
        TASK_UPD
    }task_type_t;

    class Stats {
    private:
        

        std::vector<Timer> timers;
    public:
        Stats();
        void StartTimer(uint32_t id);
        void PauseTimer(uint32_t id, bool record = false);
        void ResetTimer(uint32_t id);
        uint64_t ReportTime(uint32_t id);
        void ReportTime();

        uint64_t GetTime();
        void ResetAll();
        ~Stats();
    };


}


#endif //LEVELDB_STATS_H
