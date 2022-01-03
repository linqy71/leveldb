//
// Created by daiyi on 2020/02/02.
//

#include "timer.h"
#include <chrono>
#include <cassert>
#include <x86intrin.h>
#include <stdio.h>


namespace MyStat {

    Timer::Timer() : time_accumulated(0), started(false) {}

    void Timer::Start() {
        assert(!started);
        time_started = std::chrono::system_clock::now();
        started = true;
    }

    void Timer::Pause(bool record) {
        assert(started);

        if (record) {
            time_accumulated += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_started);
            started = false;
        } else {
            started = false;
        }
    }

    void Timer::Reset() {
        time_accumulated = std::chrono::nanoseconds(0);
        started = false;
    }

    std::chrono::nanoseconds Timer::Time() {
        //assert(!started);
        return time_accumulated;
    }
}