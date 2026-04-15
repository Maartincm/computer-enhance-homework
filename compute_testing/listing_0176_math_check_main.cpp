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
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <mmintrin.h>
#include <immintrin.h>

#include "types.h"

#include "listing_0175_math_check.cpp"

/* NOTE(casey): These are our stub functions. We will start filling
   them in with real computation over time.
*/


static f64 SqrtCE(f64 X)
{
    __m128d in = _mm_set_sd(X);
    // __m128d mask = _mm_set_sd(0);
    __m128d sqrt_res;
    sqrt_res = _mm_sqrt_sd(sqrt_res, in);
    f64 res = _mm_cvtsd_f64(sqrt_res);
    return res;
}

static inline f64 SinFin(f64 X) {
  f64 x2 = X * X;
  f64 res = 0x1.883c1c5deffbep-49;
  res = fma(x2, res, -0x1.ae43dc9bf8ba7p-41);
  res = fma(x2, res, 0x1.6123ce513b09fp-33);
  res = fma(x2, res, -0x1.ae6454d960ac4p-26);
  res = fma(x2, res, 0x1.71de3a52aab96p-19);
  res = fma(x2, res, -0x1.a01a01a014eb6p-13);
  res = fma(x2, res, 0x1.11111111110c9p-7);
  res = fma(x2, res, -0x1.5555555555555p-3);
  res = fma(x2, res, 0x1p0);
  res *= X;
  return res;
}

static inline f64 SinPos(f64 X) {
  if (X > PiOverTwo) {
    X = Pi64 - X;
  }
  f64 result = SinFin(X);
  return result;
}

static f64 SinCE(f64 X) {
  f64 result;
  if (X < 0) {
    result = -SinPos(-X);
  } else {
    result = SinPos(X);
  }
  return result;
}

static f64 CosCE(f64 X)
{
    if (X > PiOverTwo ) {
        X -= 2 * Pi64;
    }
    return SinCE(X + PiOverTwo);
}

static inline f64 ASinInner(f64 X) {
  f64 x2 = X * X;
  f64 res = 0x1.dfc53682725cap-1;
  res = fma(res, x2, -0x1.bec6daf74ed61p1);
  res = fma(res, x2, 0x1.8bf4dadaf548cp2);
  res = fma(res, x2, -0x1.b06f523e74f33p2);
  res = fma(res, x2, 0x1.4537ddde2d76dp2);
  res = fma(res, x2, -0x1.6067d334b4792p1);
  res = fma(res, x2, 0x1.1fb54da575b22p0);
  res = fma(res, x2, -0x1.57380bcd2890ep-2);
  res = fma(res, x2, 0x1.69b370aad086ep-4);
  res = fma(res, x2, -0x1.21438ccc95d62p-8);
  res = fma(res, x2, 0x1.b8a33b8e380efp-7);
  res = fma(res, x2, 0x1.c37061f4e5f55p-7);
  res = fma(res, x2, 0x1.1c875d6c5323dp-6);
  res = fma(res, x2, 0x1.6e88ce94d1149p-6);
  res = fma(res, x2, 0x1.f1c73443a02f5p-6);
  res = fma(res, x2, 0x1.6db6db3184756p-5);
  res = fma(res, x2, 0x1.3333333380df2p-4);
  res = fma(res, x2, 0x1.555555555531ep-3);
  res = fma(res, x2, 0x1p0);
  res *= X;
  return res;
}

static f64 ASinCE(f64 X) {
  f64 res;
  if (X > 1/sqrt(2)) {
    f64 NewX = sqrt(1 - X * X);
    res = PiOverTwo - ASinInner(NewX);
  } else {
    res = ASinInner(X);
  }
  return res;
}

int main(void)
{
    // CheckHardCodedReference("sin", sin, ArrayCount(RefTableSinX), RefTableSinX);
    // CheckHardCodedReference("cos", cos, ArrayCount(RefTableCosX), RefTableCosX);
    // CheckHardCodedReference("asin", asin, ArrayCount(RefTableArcSinX), RefTableArcSinX);
    // CheckHardCodedReference("sqrt", sqrt, ArrayCount(RefTableSqrtX), RefTableSqrtX);

    math_tester Tester = {};

    while(PrecisionTest(&Tester, -Pi64, Pi64))
    {
        TestResult(&Tester, sin(Tester.InputValue), SinCE(Tester.InputValue), "SinCE");
    }

    while(PrecisionTest(&Tester, -Pi64/2, Pi64/2))
    {
        TestResult(&Tester, cos(Tester.InputValue), CosCE(Tester.InputValue), "CosCE");
    }

    while(PrecisionTest(&Tester, 0, 1))
    {
        TestResult(&Tester, asin(Tester.InputValue), ASinCE(Tester.InputValue), "ASinCE");
    }

    while(PrecisionTest(&Tester, 0, 1))
    {
        TestResult(&Tester, sqrt(Tester.InputValue), SqrtCE(Tester.InputValue), "SqrtCE");
    }

    PrintResults(&Tester);

    return 0;
}
