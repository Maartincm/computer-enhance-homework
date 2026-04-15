/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

/* ========================================================================
   LISTING 176
   ======================================================================== */

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define PiOverTwo Pi64 / 2

#include "listing_0175_math_check.cpp"

/* NOTE(casey): These are our stub functions. We will start filling
   them in with real computation over time.
*/
static f64 Square(f64 X) {
  f64 Result = X * X;
  return Result;
}

static f64 SinQ(f64 X) {
  f64 X2 = Square(X);
  f64 A = -4.0 / Square(Pi64);
  f64 B = 4.0 / Pi64;

  f64 Result = A * X2 + B * X;

  return Result;
}

static f64 SinCubZeroToPiOverTwo(f64 X) {
  f64 a = -0.11592;
  f64 b = -0.0608805;
  f64 c = 1.01827;
  f64 result = a * X * Square(X) + b * Square(X) + c * X;
  return result;
}

static f64 SinCubPos(f64 X) {
  if (X > PiOverTwo) {
    X = Pi64 - X;
  }
  f64 result = SinCubZeroToPiOverTwo(X);
  return result;
}

static f64 SinCub(f64 X) {
  f64 result;
  if (X < 0) {
    result = -SinCubPos(-X);
  } else {
    result = SinCubPos(X);
  }
  return result;
}

static f64 CosCub(f64 X) {
    if (X > PiOverTwo ) {
        X -= 2 * Pi64;
    }
    return SinCub(X + PiOverTwo);
}

int main(void) {
  f64 LowerBounds[] = {0, -Pi64};
  for (u32 BoundIndex = 0; BoundIndex < ArrayCount(LowerBounds); ++BoundIndex) {
    math_tester Tester = {};

    f64 LowerBound = LowerBounds[BoundIndex];
    printf("RANGE: [%+.24f, %+.24f]\n", LowerBound, Pi64);

    while (PrecisionTest(&Tester, LowerBound, Pi64)) {
      f64 RefOutput = sin(Tester.InputValue);
      TestResult(&Tester, RefOutput, SinQ(Tester.InputValue), "SinQ");
      TestResult(&Tester, RefOutput, SinCub(Tester.InputValue), "SinCub");
    }

    while (PrecisionTest(&Tester, LowerBound, Pi64)) {
      f64 RefOutput = cos(Tester.InputValue);
      TestResult(&Tester, RefOutput, CosCub(Tester.InputValue), "CosCub");
    }

    PrintResults(&Tester);
    printf("\n");
  }

  return 0;
}
