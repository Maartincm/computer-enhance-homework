/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

/* ========================================================================
   LISTING 72
   ======================================================================== */

#include <stdint.h>
#include <stdio.h>

#include "haversine_types.h"

#include "haversine_timing.cpp"

int main(void)
{
	u64 OSFreq = GetOSTimerFreq();
	printf("    OS Freq: %lu\n", OSFreq);

	u64 CPUStart = ReadCPUTimer();
	u64 OSStart = ReadOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	while(OSElapsed < OSFreq)
	{
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}

	u64 CPUEnd = ReadCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;

	printf("   OS Timer: %lu -> %lu = %lu elapsed\n", OSStart, OSEnd, OSElapsed);
	printf(" OS Seconds: %.4f\n", (f64)OSElapsed/(f64)OSFreq);

	printf("  CPU Timer: %lu -> %lu = %lu elapsed\n", CPUStart, CPUEnd, CPUElapsed);

    return 0;
}
