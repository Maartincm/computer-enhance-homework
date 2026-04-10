#pragma once
#include <cstdint>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

