//
// Created by daiyi on 2019/09/30.
//

#include <cassert>
#include "stats.h"
#include <cmath>
#include <iostream>
#include <x86intrin.h>

using std::stoull;

namespace MyStat {
    float reference_frequency = 2.6;

    Stats::Stats() : timers(6, Timer{}) {}

    void Stats::StartTimer(uint32_t id) {
        Timer& timer = timers[id];
        timer.Start();
    }

    void Stats::PauseTimer(uint32_t id, bool record) {
        Timer& timer = timers[id];
        return timer.Pause(record);
    }

    void Stats::ResetTimer(uint32_t id) {
        Timer& timer = timers[id];
        timer.Reset();
    }

    // uint64_t Stats::ReportTime(uint32_t id) {
    //     Timer& timer = timers[id];
    //     return timer.Time();
    // }

    void Stats::ReportTime() {
        for (int i = 0; i < timers.size(); ++i) {
            auto duration = timers[i].Time().count();
            std::cout << "Timer " << i << ": " << duration / (1000 * 1000) << " ms" << std::endl;
        }
    }

    // uint64_t Stats::GetTime() {
    //     unsigned int dummy = 0;
    //     uint64_t time_elapse = __rdtscp(&dummy) - initial_time;
    //     return time_elapse / reference_frequency;
    // }


    void Stats::ResetAll() {
        for (Timer& t: timers) t.Reset();
    }

    Stats::~Stats() {
        ReportTime();
    }

}





