#include "haversine_types.h"

#include "haversine_timing.cpp"

u64 wowza = 0;
u64 wawza = 0;

void recursive_func();

void one_level_func() {
  TIMED_FUNC;
  for (u64 i = 0; i < 1<<30; i++) {
    wowza += 11;
    wowza *= 17;
  }
}

void not_called() {
  TIMED_FUNC;
  return;
}

void inner_func() {
  TIMED_FUNC;
  for (u64 i = 0; i < 1<<4; i++) {
    wowza += 11;
    wowza *= 17;
  }

}

void two_level_func() {
  TIMED_FUNC;
  for (u64 i = 0; i < 1<<25; i++) {
    wowza += 11;
    wowza *= 17;
    inner_func();
  }
}

void recursive_ping_pong_func() {
  for (u64 i = 0; i < 1<<28; i++) {
    wowza += 11;
    wowza *= 17;
  }
  recursive_func();
}

void recursive_func() {
  TIMED_FUNC;
  wawza--;
  for (u64 i = 0; i < 1<<28; i++) {
    wowza += 11;
    wowza *= 17;
  }
  if (wawza) {
    recursive_ping_pong_func();
  }
  return;
}

int main(int argc, char *argv[]) {
  Timing::begin_profiler();

  one_level_func();
  two_level_func();

  wawza = 1<<4;
  recursive_func();

  Timing::print_profiling();
}
