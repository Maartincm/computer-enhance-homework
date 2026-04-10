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

  u64 SizesArray[4] = {16 * 1024, 256 * 1024, 1 * 1024 * 1024,
                       32 * 1024 * 1024};
  u8 UnalignedOffsetArray[14] = {0, 1, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65};
  repetition_tester
      Testers[ArrayCount(SizesArray) * ArrayCount(UnalignedOffsetArray)] = {};
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

    for (u32 SizeIndex = 0; SizeIndex < ArrayCount(SizesArray); ++SizeIndex) {
      u64 BytesToRead = SizesArray[SizeIndex];
      for (u32 UnalignedOffsetIndex = 0;
           UnalignedOffsetIndex < ArrayCount(UnalignedOffsetArray);
           UnalignedOffsetIndex++) {
        repetition_tester *Tester =
            Testers + SizeIndex * ArrayCount(UnalignedOffsetArray) +
            UnalignedOffsetIndex;
        u8 AlignmentOffset = UnalignedOffsetArray[UnalignedOffsetIndex];
        printf("\n--- Read32x8 of %luk alignment offset +%u ---\n",
               BytesToRead / 1024, AlignmentOffset);
        NewTestWave(Tester, VirtualReadSize, GetCPUTimerFreq());
        u64 RepetitionCount = VirtualReadSize / BytesToRead;

        while (IsTesting(Tester)) {
          BeginTime(Tester);
          Read_32x8(BytesToRead, Buffer.Data + AlignmentOffset, RepetitionCount);
          EndTime(Tester);
          CountBytes(Tester, VirtualReadSize);
        }
      }
    }

  } else {
    fprintf(stderr, "Unable to allocate memory buffer for testing\n");
  }

  FreeBuffer(&Buffer);

  return 0;
}
