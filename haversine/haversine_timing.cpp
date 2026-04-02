#include <cstdio>
#include <ctime>

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

u64 get_os_timer_frequency() { return 1000000000; }

u64 read_os_timer() {
  timespec tp;
  if (clock_gettime(CLOCK_MONOTONIC, &tp) != -1) {
    return get_os_timer_frequency() * (u64)tp.tv_sec + (u64)tp.tv_nsec;
  }
  return 0;
}

inline u64 read_cpu_timer() { return __rdtsc(); }

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

u64 GetOSTimerFreq() { return get_os_timer_frequency(); }

u64 ReadOSTimer() { return read_os_timer(); }

u64 ReadCPUTimer() { return read_cpu_timer(); }

// void timing_start(TimeMeasurement *timings) {
//     timings->start_cpu = std::clock();
//     timings->start_wall = std::chrono::steady_clock::now();
// }
//
// void timing_end(TimeMeasurement *timings) {
//     timings->end_cpu = std::clock();
//     timings->end_wall = std::chrono::steady_clock::now();
//     timings->elapsed_wall_ms = std::chrono::duration<f64,
//     std::milli>(timings->end_wall - timings->start_wall).count();
//     timings->elapsed_cpu_ms = 1000.0 * (timings->end_cpu -
//     timings->start_cpu) / CLOCKS_PER_SEC;
// }

void timing_start(TimeMeasurement *timings) {
  timings->start_cpu = read_cpu_timer();
  timings->start_wall = read_os_timer();
}

void timing_end(TimeMeasurement *timings) {
  timings->end_cpu = read_cpu_timer();
  timings->end_wall = read_os_timer();
  timings->elapsed_cpu_ticks = timings->end_cpu - timings->start_cpu;
  timings->elapsed_wall_ms = 1000.0 *
                             (timings->end_wall - timings->start_wall) /
                             get_os_timer_frequency();
  timings->elapsed_cpu_ms =
      1000.0 * (timings->end_cpu - timings->start_cpu) / guess_cpu_frequency();
}

#define TIMED_BLOCK(name) Timing::DeferredTimer _(name, __COUNTER__ + 1)
#define TIMED_FUNC TIMED_BLOCK(__func__)

namespace Timing {
struct TimedScope {
  const char *scope_name;
  u64 ticks;
  u64 elapsed;
  u64 elapsed_in_children;
  u32 times_called;
  s32 parent_scope_index;
};

struct GlobalTiming {
  TimedScope timing_slots[64];
  u64 begin_tick;
  s32 current_scope_index;
  s32 slot_count;
};

static GlobalTiming global_timing;

struct DeferredTimer {
  u64 _scope_index;
  DeferredTimer(const char *scope_name, u64 scope_index)
      : _scope_index(scope_index) {
    TimedScope *slot = global_timing.timing_slots + scope_index;
    if (!slot->ticks) {
      slot->ticks = read_cpu_timer();
      slot->scope_name = scope_name;
      slot->parent_scope_index = global_timing.current_scope_index;
    }
    slot->times_called++;
    global_timing.current_scope_index = scope_index;
  }
  ~DeferredTimer() {
    TimedScope *slot = global_timing.timing_slots + _scope_index;
    if (slot->ticks) {
      u64 elapsed = read_cpu_timer() - slot->ticks;
      slot->ticks = 0;
      slot->elapsed += elapsed;
      TimedScope *parent_slot = global_timing.timing_slots + slot->parent_scope_index;
      parent_slot->elapsed_in_children += elapsed;
    }
    global_timing.current_scope_index = slot->parent_scope_index;
  }
};

void begin_profiler() { global_timing.begin_tick = read_cpu_timer(); }

void print_profiling() {
  u64 cpu_freq = guess_cpu_frequency();
  u64 total_elapsed = read_cpu_timer() - global_timing.begin_tick;
  printf("====================================================================="
         "===========\n");
  printf("Profiling info:\n");
  printf("  Total time taken: (CPU: %.4f ms | ticks: %lu | %2.2f%%)\n",
         1000.0 * (f64)total_elapsed / cpu_freq, total_elapsed, 100.0);
  for (u32 i = 0; i < 64; i++ ) {
    TimedScope *timing_slot = global_timing.timing_slots + i;
    if (!timing_slot->times_called) {
      continue;
    }
    u64 ticks = timing_slot->elapsed - timing_slot->elapsed_in_children;
    printf("  [%s x%u]: (CPU: %.4f ms | ticks: %lu | "
           "%2.2f%%)",
           timing_slot->scope_name, timing_slot->times_called, 1000.0 * (f64)ticks / cpu_freq, ticks,
           100.0 * ticks / total_elapsed);
    if (timing_slot->elapsed_in_children) {
      u64 ticks_plus_children = timing_slot->elapsed;
      printf(" (plus children: %.4f ms | %2.2f%%)",
             1000.0 * (f64)ticks_plus_children / cpu_freq,
             100.0 * (f64)ticks_plus_children / total_elapsed);
    }
    printf("\n");
  }
  printf("====================================================================="
         "===========\n");
}
} // namespace Timing
