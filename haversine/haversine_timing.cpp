#include <chrono>
#include <ctime>
#include <ratio>

#include "haversine_types.h"

struct TimeMeasurement {
    std::chrono::time_point<std::chrono::steady_clock> start_wall;
    std::chrono::time_point<std::chrono::steady_clock> end_wall;
    std::clock_t start_cpu;
    std::clock_t end_cpu;
    f64 elapsed_wall_ms;
    f64 elapsed_cpu_ms;
};

void timing_start(TimeMeasurement *timings) {
    timings->start_cpu = std::clock();
    timings->start_wall = std::chrono::steady_clock::now();
}

void timing_end(TimeMeasurement *timings) {
    timings->end_cpu = std::clock();
    timings->end_wall = std::chrono::steady_clock::now();
    timings->elapsed_wall_ms = std::chrono::duration<f64, std::milli>(timings->end_wall - timings->start_wall).count();
    timings->elapsed_cpu_ms = 1000.0 * (timings->end_cpu - timings->start_cpu) / CLOCKS_PER_SEC;
}

