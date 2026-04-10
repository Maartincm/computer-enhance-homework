#pragma once

#include <cstdio>
#include <ctime>

#include <x86intrin.h>

#include "haversine_types.h"

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

namespace Timing {

struct TimedScope {
  const char *scope_name;
  u64 elapsed_exclusive;
  u64 elapsed_inclusive;
  u64 mem_bytes_transferred;
  u32 times_called;
};

struct GlobalTiming {
  TimedScope timing_slots[64];
  u64 begin_tick;
  s32 current_scope_index;
};
static GlobalTiming global_timing;

#if PROFILE
#define TIMED_BLOCK_BANDWIDTH(name, size) Timing::DeferredTimer _(name, __COUNTER__ + 1, size)
#define TIMED_BLOCK(name) TIMED_BLOCK_BANDWIDTH(name, 0)
#define TIMED_FUNC TIMED_BLOCK(__func__)
#define TIMED_FUNC_BANDWIDTH(size) TIMED_BLOCK_BANDWIDTH(__func__, size)

struct DeferredTimer {
  const char *_scope_name;
  u64 _initial_elapsed;
  u64 _begin;
  u64 _mem_bytes_transferred;
  u32 _scope_index;
  s32 _parent_scope_index;

  DeferredTimer(const char *scope_name, u64 scope_index, u64 size) {
    _scope_index = scope_index;
    TimedScope *slot = global_timing.timing_slots + _scope_index;
    _scope_name = scope_name;
    _initial_elapsed = slot->elapsed_inclusive;
    _begin = read_cpu_timer();
    _mem_bytes_transferred = size;
    _parent_scope_index = global_timing.current_scope_index;
    global_timing.current_scope_index = scope_index;
  }
  ~DeferredTimer() {
    u64 elapsed = read_cpu_timer() - _begin;
    TimedScope *slot = global_timing.timing_slots + _scope_index;
    TimedScope *parent_slot = global_timing.timing_slots + _parent_scope_index;
    slot->scope_name = _scope_name;
    slot->elapsed_exclusive += elapsed;
    slot->elapsed_inclusive = _initial_elapsed + elapsed;
    slot->mem_bytes_transferred += _mem_bytes_transferred;
    slot->times_called++;
    parent_slot->elapsed_exclusive -= elapsed;
    global_timing.current_scope_index = _parent_scope_index;
  }
};

#else
#define TIMED_BLOCK(...)
#define TIMED_BLOCK_BANDWIDTH(...)
#define TIMED_FUNC
#define TIMED_FUNC_BANDWIDTH(...)
#endif

void begin_profiler() { global_timing.begin_tick = read_cpu_timer(); }

void print_profiling() {
  u64 cpu_freq = guess_cpu_frequency();
  u64 total_elapsed = read_cpu_timer() - global_timing.begin_tick;
  printf("====================================================================="
         "===========\n");
  printf("Profiling info (CPU Freq: %.4f GHz):\n", (f64)cpu_freq / 1000000000.0);
  printf("  Total time taken: (CPU: %.4f ms | ticks: %lu | %2.2f%%)\n",
         1000.0 * (f64)total_elapsed / cpu_freq, total_elapsed, 100.0);
  for (u32 i = 0; i < 64; i++ ) {
    TimedScope *timing_slot = global_timing.timing_slots + i;
    if (!timing_slot->times_called) {
      continue;
    }
    u64 ticks = timing_slot->elapsed_exclusive;
    printf("  [%s x%u]: (CPU: %.4f ms | ticks: %lu | "
           "%2.2f%%)",
           timing_slot->scope_name, timing_slot->times_called, 1000.0 * (f64)ticks / cpu_freq, ticks,
           100.0 * ticks / total_elapsed);
    if (timing_slot->elapsed_inclusive - timing_slot->elapsed_exclusive) {
      u64 ticks_plus_children = timing_slot->elapsed_inclusive;
      printf(" (plus children: %.4f ms | %2.2f%%)",
             1000.0 * (f64)ticks_plus_children / cpu_freq,
             100.0 * (f64)ticks_plus_children / total_elapsed);
    }
    if (timing_slot->mem_bytes_transferred) {
      f64 elapsed_in_seconds = (f64)timing_slot->elapsed_inclusive / cpu_freq;
      f64 transfer_mb = (f64)timing_slot->mem_bytes_transferred / 1024.0 / 1024.0;
      f64 throughput_gb = (f64)timing_slot->mem_bytes_transferred / 1024.0 / 1024.0 / 1024.0 / elapsed_in_seconds;
      printf(" (transferred %.4f MB at %.4f GB/s)", transfer_mb, throughput_gb);
    }
    printf("\n");
  }
  printf("====================================================================="
         "===========\n");
}
} // namespace Timing
