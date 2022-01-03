//
// Created by daiyi on 2020/02/02.
//

#ifndef LEVELDB_TIMER_H
#define LEVELDB_TIMER_H


#include <cstdint>
#include <ctime>
#include <utility>
#include <vector>
#include <chrono>
using namespace std::chrono;

namespace MyStat {

    class Timer {
        system_clock::time_point time_started;
        std::chrono::nanoseconds time_accumulated;
        bool started;

    public:
        void Start();
        void Pause(bool record = false);
        void Reset();
        std::chrono::nanoseconds Time();

        Timer();
        ~Timer() = default;
    };

}


#endif //LEVELDB_TIMER_H
