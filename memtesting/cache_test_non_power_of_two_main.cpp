/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

/* ========================================================================
   LISTING 153
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.

   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "types.h"

#include "listing_0125_buffer.cpp"
#include "listing_0137_os_platform.cpp"
#include "listing_0109_pagefault_repetition_tester.cpp"

extern "C" void Read_32x8(u64 TestSize, u8 *Data, u64 RepetitionCount);
// #pragma comment (lib, "listing_0152_cache_test")

int main(void) {
  InitializeOSPlatform();

  u64 SizesArray[30 * 16] = {};
  repetition_tester Testers[30 * 16] = {};
  u32 MinSizeIndex = 15;
  u32 MaxRegionIndex = 26;
  u64 VirtualReadSize = 1ull << 30; // 1GB worth of reads

  buffer Buffer = AllocateBuffer(VirtualReadSize);
  if (IsValid(Buffer)) {
    // NOTE(casey): Because OSes may not map allocated pages until they are
    // written to, we write garbage to the entire buffer to force it to be
    // mapped.
    u64 PrePageFaultCount = ReadOSPageFaultCount();
    for (u64 ByteIndex = 0; ByteIndex < Buffer.Count; ++ByteIndex) {
      Buffer.Data[ByteIndex] = (u8)ByteIndex;
    }
    u64 PageFaultCount = ReadOSPageFaultCount() - PrePageFaultCount;
    printf("Faulted %lu times while touching %lu bytes", PageFaultCount,
           Buffer.Count);

    for (u32 SizeIndex = MinSizeIndex; SizeIndex < MaxRegionIndex;
         ++SizeIndex) {

      u64 RegionCenter = (1ull << SizeIndex);
      for (s32 MiniChunkOffset = -4; MiniChunkOffset < 8; MiniChunkOffset++) {
        u32 Index = SizeIndex * 16 + MiniChunkOffset;
        repetition_tester *Tester = Testers + Index;
        // Sample 1k offsets before and after power of two regions
        u64 RegionSize = RegionCenter + MiniChunkOffset * 1024;
        u64 RepetitionCount = VirtualReadSize / RegionSize;
        u64 ActualReadSize = RepetitionCount * RegionSize;
        SizesArray[Index] = RegionSize;
        printf("\n--- Read32x8 of %luk ---\n", RegionSize / 1024);
        NewTestWave(Tester, ActualReadSize, GetCPUTimerFreq(), 5);

        while (IsTesting(Tester)) {
          BeginTime(Tester);
          Read_32x8(RegionSize, Buffer.Data, RepetitionCount);
          EndTime(Tester);
          CountBytes(Tester, ActualReadSize);
        }
      }
    }

    printf("Region Size,gb/s\n");
    for (u32 SizeIndex = MinSizeIndex; SizeIndex < ArrayCount(Testers);
         ++SizeIndex) {
      repetition_tester *Tester = Testers + SizeIndex;
      if (Tester->Mode == TestMode_Completed) {
        repetition_value Value = Tester->Results.Min;
        f64 Seconds = SecondsFromCPUTime((f64)Value.E[RepValue_CPUTimer],
                                         Tester->CPUTimerFreq);
        f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
        f64 Bandwidth = Value.E[RepValue_ByteCount] / (Gigabyte * Seconds);

        printf("%lu,%f\n", SizesArray[SizeIndex], Bandwidth);
      }
    }
  } else {
    fprintf(stderr, "Unable to allocate memory buffer for testing\n");
  }

  FreeBuffer(&Buffer);

  return 0;
}
