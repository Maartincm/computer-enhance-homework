#include <chrono>
#include <ctime>
#include <ratio>

#include <x86intrin.h>

#include "haversine_types.h"

struct TimeMeasurement {
    // std::chrono::time_point<std::chrono::steady_clock> start_wall;
    // std::chrono::time_point<std::chrono::steady_clock> end_wall;
    // std::clock_t start_cpu;
    // std::clock_t end_cpu;
    u64 start_wall;
    u64 end_wall;
    u64 start_cpu;
    u64 end_cpu;
    f64 elapsed_wall_ms;
    f64 elapsed_cpu_ms;
    u64 elapsed_cpu_ticks;
};


u64 get_os_timer_frequency() {
    return 1000000000;
}

u64 read_os_timer() {
    timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != -1) {
        return get_os_timer_frequency()*(u64)tp.tv_sec + (u64)tp.tv_nsec;
    }
    return 0;
}

inline
u64 read_cpu_timer() {
    return __rdtsc();
}

u64 guess_cpu_frequency() {
    u64 milliseconds_to_wait = 10;
    u64 os_timer_freq = get_os_timer_frequency();
    u64 os_timer_start = read_os_timer();
    u64 os_timer_elapsed = 0;
    u64 time_to_wait_in_os_unit = os_timer_freq * milliseconds_to_wait / 1000;
    u64 cpu_timer_start = read_cpu_timer();
    while (os_timer_elapsed < time_to_wait_in_os_unit) {
        os_timer_elapsed = read_os_timer() - os_timer_start;
    }
    u64 cpu_timer_end = read_cpu_timer();
    return os_timer_freq * (cpu_timer_end - cpu_timer_start) / os_timer_elapsed;
}

u64 GetOSTimerFreq() {
    return get_os_timer_frequency();
}

u64 ReadOSTimer() {
    return read_os_timer();
}

u64 ReadCPUTimer() {
    return read_cpu_timer();
}

// void timing_start(TimeMeasurement *timings) {
//     timings->start_cpu = std::clock();
//     timings->start_wall = std::chrono::steady_clock::now();
// }
//
// void timing_end(TimeMeasurement *timings) {
//     timings->end_cpu = std::clock();
//     timings->end_wall = std::chrono::steady_clock::now();
//     timings->elapsed_wall_ms = std::chrono::duration<f64, std::milli>(timings->end_wall - timings->start_wall).count();
//     timings->elapsed_cpu_ms = 1000.0 * (timings->end_cpu - timings->start_cpu) / CLOCKS_PER_SEC;
// }

void timing_start(TimeMeasurement *timings) {
    timings->start_cpu = read_cpu_timer();
    timings->start_wall = read_os_timer();
}

void timing_end(TimeMeasurement *timings) {
    timings->end_cpu = read_cpu_timer();
    timings->end_wall = read_os_timer();
    timings->elapsed_cpu_ticks = timings->end_cpu - timings->start_cpu;
    timings->elapsed_wall_ms = 1000.0 * (timings->end_wall - timings->start_wall) / get_os_timer_frequency();
    timings->elapsed_cpu_ms = 1000.0 * (timings->end_cpu - timings->start_cpu) / guess_cpu_frequency();
}
