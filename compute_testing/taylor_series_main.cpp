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

#include <emmintrin.h>
#include <math.h>
#include <cmath>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>

#include "types.h"

#include "generated_series_horner.cpp"
#include "generated_series_taylor.cpp"
#include "listing_0175_math_check.cpp"

static f64 ArcsineRadiansC_Taylor[] =
{
    1.0,
    0.1666666666666666666666666666666666666666666666666666666666666666666666666666667,
    0.075,
    0.04464285714285714285714285714285714285714285714285714285714285714285714285714286,
    0.03038194444444444444444444444444444444444444444444444444444444444444444444444444,
    0.02237215909090909090909090909090909090909090909090909090909090909090909090909091,
    0.01735276442307692307692307692307692307692307692307692307692307692307692307692308,
    0.01396484375,
    0.01155180089613970588235294117647058823529411764705882352941176470588235294117647,
    0.009761609529194078947368421052631578947368421052631578947368421052631578947368421,
    0.008390335809616815476190476190476190476190476190476190476190476190476190476190476,
    0.007312525873598845108695652173913043478260869565217391304347826086956521739130435,
    0.0064472103118896484375,
    0.005740037670841923466435185185185185185185185185185185185185185185185185185185185,
    0.005153309682319904195851293103448275862068965517241379310344827586206896551724138,
    0.004660143486915096159904233870967741935483870967741935483870967741935483870967742,
    0.004240907093679363077337091619318181818181818181818181818181818181818181818181818,
    0.003880964558837669236319405691964285714285714285714285714285714285714285714285714,
    0.003569205393825934545413867847339527027027027027027027027027027027027027027027027,
    0.003297059503473484745392432579627403846153846153846153846153846153846153846153846,
    0.003057821649258030669354810947325171493902439024390243902439024390243902439024390,
    0.002846178401108942167876764785411746002906976744186046511627906976744186046511628,
    0.002657870638207289933537443478902180989583333333333333333333333333333333333333333,
    0.002489448678246883494640759965206714386635638297872340425531914893617021276595745,
    0.002338091892111975186930883827866340170101243622448979591836734693877551020408163,
    0.002201473973710138205420020419885130489573759191176470588235294117647058823529412,
    0.002077661032518167442778473553019312192808906987028301886792452830188679245283019,
    0.001965033616277283618439303774555975740606134588068181818181818181818181818181818,
    0.001862226406403127489279102104646562222848858749657346491228070175438596491228070,
};

static f64 ArcsineRadiansC_MFTWP[][22] =
{
    // NOTE(casey): This minimax coefficient table was donated by Demetri Spanos

    {},
    {},
    {0x1.fdfcefbdd3154p-1 , 0x1.c427597754a37p-3},
    {0x1.0019e5b9a7693p0, 0x1.3b5f83d579a47p-3, 0x1.0da162d6fae3dp-3},
    {0x1.fffa004bed736p-1, 0x1.5acaca323d3aep-3, 0x1.a52ade47d967dp-5, 0x1.b0931e5a07f25p-4},
    {0x1.0000609783343p0, 0x1.543eb056cd449p-3, 0x1.52db50c86c17fp-4, 0x1.c36707c70d21cp-8, 0x1.8faf3815344ddp-4},
    {0x1.ffffe6586d628p-1, 0x1.558b2dc0be61cp-3, 0x1.2a2202ec5cb8p-4, 0x1.f96fb970de571p-5, -0x1.ac22c3939a9a9p-6, 0x1.912219085f248p-4},
    {0x1.000001c517503p0, 0x1.554b23dabce0bp-3, 0x1.35948cff7046bp-4, 0x1.391ca703d0d07p-5, 0x1.03149c11e9277p-4, -0x1.e523c15fbf438p-5, 0x1.a906b64a9bdc7p-4},
    {0x1.ffffff7f5bbbcp-1, 0x1.55573c38fd397p-3, 0x1.329cc1329ab48p-4, 0x1.7f40be8459b49p-5, 0x1.ea3ebd68f4abp-7, 0x1.4b1dad0e7b7e5p-4, -0x1.912818d0401bfp-4, 0x1.d3ec796e5ec8bp-4},
    {0x1.00000009557b8p0, 0x1.5554fb74b4bffp-3, 0x1.3356afe11c66p-4, 0x1.685b6636af595p-5, 0x1.2c25059387c7ep-5, -0x1.5b983df09138p-7, 0x1.db45568eb9217p-4, -0x1.2c7c4453a9b3ep-3, 0x1.0903eea6d1357p-3},
    {0x1.fffffffd3e442p-1, 0x1.555565c9beb43p-3, 0x1.332b1eab3d6f2p-4, 0x1.6f3ed264eef28p-5, 0x1.cc5fc8ed87fdbp-6, 0x1.38c39ea555adep-5, -0x1.88322c8ce661fp-5, 0x1.656a2ea43451dp-3, -0x1.aed7e0dfdbd27p-3, 0x1.32de0e0b3820fp-3},
    {0x1.0000000034db9p0, 0x1.5555525723f64p-3, 0x1.3334fd1dd69f5p-4, 0x1.6d4c8c3659p-5, 0x1.fe5b240c320ebp-6, 0x1.0076fe3314273p-6, 0x1.b627b3be92bd4p-5, -0x1.ba657aa72abeep-4, 0x1.103aa8bb00a4ep-2, -0x1.2deb335977b56p-2, 0x1.699a7715830d2p-3},
    {0x1.ffffffffeff9dp-1, 0x1.555555dff5e06p-3, 0x1.3332d0221f548p-4, 0x1.6dd27e8c33d52p-5, 0x1.edd05e3dff008p-6, 0x1.992d0b8b03f01p-6, -0x1.ac779f1be0507p-13, 0x1.73bb5b359003ap-4, -0x1.a8326f2354f8ap-3, 0x1.9e1b8885f9661p-2, -0x1.a1b0aa236a282p-2, 0x1.b038b25a40e08p-3},
    {0x1.00000000013ap0, 0x1.5555553c5c8a7p-3, 0x1.33334839e1acap-4, 0x1.6dafeb7453ee6p-5, 0x1.f2f65baf85a8cp-6, 0x1.5f396c79d5687p-6, 0x1.9a8031b47fd85p-6, -0x1.cbd84d319158p-6, 0x1.53df7e2c17602p-3, -0x1.7a954b7cb46e6p-2, 0x1.38e97b1392a69p-1, -0x1.1eabdc3fe561ap-1, 0x1.056424720e768p-2},
    {0x1.ffffffffff9fp-1, 0x1.55555559d0d4p-3, 0x1.33332ecf01c13p-4, 0x1.6db88c4cfe8eap-5, 0x1.f17068ec7ac68p-6, 0x1.73b9408ccb9b1p-6, 0x1.d2a82629eb78ep-7, 0x1.1b4dda11bb1d2p-5, -0x1.5210c527bd7ep-4, 0x1.3a638b5965e45p-2, -0x1.4434b98838c1dp-1, 0x1.d52ccc09ba2cdp-1, -0x1.8792b45ef365ep-1, 0x1.3f5545e9e11eap-2},
    {0x1.0000000000079p0, 0x1.5555555487dd3p-3, 0x1.3333341adb0b8p-4, 0x1.6db67483a8f77p-5, 0x1.f1defdcf41a11p-6, 0x1.6ce213041c326p-6, 0x1.2f8bd23b33763p-6, 0x1.34a6d9f27428dp-8, 0x1.007f36ef69d66p-4, -0x1.850e0d65729e1p-3, 0x1.1f42350f23ccep-1, -0x1.0e0b5512f8d35p0, 0x1.5d065bf34c03ep0, -0x1.0a98c5604a5c6p0, 0x1.8978c6502660ap-2},
    {0x1.fffffffffffdap-1, 0x1.555555557a085p-3, 0x1.33333304070d3p-4, 0x1.6db6f35f4ac13p-5, 0x1.f1c0bdf8248f6p-6, 0x1.6f0e61397193p-6, 0x1.15740f26a5e24p-6, 0x1.24069344266aap-6, -0x1.c02ef74c5e655p-7, 0x1.07833aeac1562p-3, -0x1.97487ee8ceb5p-2, 0x1.0178f7f5c01bdp0, -0x1.b8b2ea879a2a5p0, 0x1.01ccfbe6e1f6ap1, -0x1.6a46d11c16386p0, 0x1.e86a774524862p-2},
    {0x1.0000000000003p0, 0x1.555555554ecb4p-3, 0x1.3333333cb4a27p-4, 0x1.6db6d5f669d29p-5, 0x1.f1c8c3485860bp-6, 0x1.6e64f7828f426p-6, 0x1.1ea1dc340da9p-6, 0x1.98123c756ff58p-7, 0x1.7a0c83f514b22p-6, -0x1.bd6eb7cdaf8e4p-5, 0x1.162743c14bf13p-2, -0x1.944c737b04ef5p-1, 0x1.c47bd23ee68a2p0, -0x1.61d1590acbbfp1, 0x1.7a6e0b194804dp1, -0x1.eba481b8f24dfp0, 0x1.311805d4c6d33p-1},
    {0x1.fffffffffffffp-1, 0x1.555555555683fp-3, 0x1.3333333148aa7p-4, 0x1.6db6dca9f82d4p-5, 0x1.f1c6b0ea300d7p-6, 0x1.6e96be6dbe49ep-6, 0x1.1b8cc838ee86ep-6, 0x1.dc086c5d99cdcp-7, 0x1.b1b8d27cd7e72p-8, 0x1.5565a3d3908b9p-5, -0x1.2ab04ba9012e3p-3, 0x1.224c4dbe13cbdp-1, -0x1.83633c76e4551p0, 0x1.86bbff2a6c7b6p1, -0x1.188f223fe5f34p2, 0x1.14672d35db97ep2, -0x1.4d84801ff1aa1p1, 0x1.7f820d52c2775p-1},
    {0x1p0, 0x1.555555555531ep-3, 0x1.3333333380df2p-4, 0x1.6db6db3184756p-5, 0x1.f1c73443a02f5p-6, 0x1.6e88ce94d1149p-6, 0x1.1c875d6c5323dp-6, 0x1.c37061f4e5f55p-7, 0x1.b8a33b8e380efp-7, -0x1.21438ccc95d62p-8, 0x1.69b370aad086ep-4, -0x1.57380bcd2890ep-2, 0x1.1fb54da575b22p0, -0x1.6067d334b4792p1, 0x1.4537ddde2d76dp2, -0x1.b06f523e74f33p2, 0x1.8bf4dadaf548cp2, -0x1.bec6daf74ed61p1, 0x1.dfc53682725cap-1},
    {0x1p0, 0x1.55555555555bap-3, 0x1.3333333323ebcp-4, 0x1.6db6db7adc18bp-5, 0x1.f1c716a8f3363p-6, 0x1.6e8c66fac48d5p-6, 0x1.1c3da3ac97e63p-6, 0x1.cbb180b74d85dp-7, 0x1.62b81445afbfdp-7, 0x1.050a65cdec399p-6, -0x1.018ae6d82506cp-5, 0x1.a361973086e84p-3, -0x1.7f8907c1978c3p-1, 0x1.1debe7d3f064p1, -0x1.411c99c675e12p2, 0x1.106a078008a9ap3, -0x1.500975aa37fb8p3, 0x1.1ea75d01d0debp3, -0x1.2ee507d6a1a5fp2, 0x1.3070aa6a5b88ep0},
    {0x1p0, 0x1.5555555555544p-3, 0x1.3333333336209p-4, 0x1.6db6db6aeb726p-5, 0x1.f1c71dcf049c4p-6, 0x1.6e8b6f8df785cp-6, 0x1.1c53c3234c54p-6, 0x1.c8eb3e8133ceap-7, 0x1.8335ee4136147p-7, 0x1.d9a5ff05f747ep-8, 0x1.b949ad43fb2bdp-6, -0x1.9080df821c302p-4, 0x1.e245cd46c886cp-2, -0x1.99434e2a3223ap0, 0x1.147d4d3b7ec76p2, -0x1.1e2a8ce097204p3, 0x1.c17aa6abf54eap3, -0x1.02778b2d86e57p4, 0x1.9ccd7e4c0706bp3, -0x1.9a13424bd53c2p2, 0x1.837ec3890fee1p0},
    {0x1p0, 0x1.5555555555558p-3, 0x1.3333333332aedp-4, 0x1.6db6db6e45234p-5, 0x1.f1c71c24301p-6, 0x1.6e8baf9ddc763p-6, 0x1.1c4d64d353371p-6, 0x1.c9cf1f8de89e6p-7, 0x1.778d723247697p-7, 0x1.5fcac651d07d4p-7, 0x1.799c2f33c0274p-12, 0x1.e288894a8bc33p-5, -0x1.0446ef7fdb149p-2, 0x1.0ba0fa7048fb2p0, -0x1.a273e0e74ee85p1, 0x1.034f776a3db58p3, -0x1.f1adf47b08719p3, 0x1.6c271c319b92ap4, -0x1.886f83ada1ccfp4, 0x1.26c247c3a321bp4, -0x1.146482ddd5f29p3, 0x1.ed3ada8793e41p0},
};

static f64 SineRadiansC_Taylor[16] =
{
    1.0,
    -0.1666666666666666666666666666666666666666666666666666666666666666666666666666667, // 1 / 3!
    0.008333333333333333333333333333333333333333333333333333333333333333333333333333333, // 1 / 5!
    -0.0001984126984126984126984126984126984126984126984126984126984126984126984126984127, // 1 / 7!

    2.755731922398589065255731922398589065255731922398589065255731922398589065255732e-6, // 1 / 9!
    -2.505210838544171877505210838544171877505210838544171877505210838544171877505211e-8, // 1 / 11!
    1.605904383682161459939237717015494793272571050348828126605904383682161459939238e-10, // 1 / 13!
    -7.647163731819816475901131985788070444155100239756324412409068493724578380663037e-13, // 1 / 15!

    2.811457254345520763198945583010320016233492735204531033973922240339918522302587e-15, // 1 / 17!
    -8.220635246624329716955981236872280749220738991826114134426673217368182813750255e-18, // 1 / 19!
    1.957294106339126123084757437350543035528747379006217651053969813659091146131013e-20, // 1 / 21!
    -3.868170170630684037716911931522812323179342646257347136470296074425081316464453e-23, // 1 / 23!

    6.446950284384473396194853219204687205298904410428911894117160124041802194107421e-26, // 1 / 25!
    -9.183689863795546148425716836473913397861687194343179336349230945928493153999175e-29, // 1 / 27!
    1.130996288644771693155876457693831699244050147086598440437097407134050881034381e-31, // 1 / 29!
    -1.216125041553517949629974685692292149724785104394191871437739147455968689284281e-34, // 1 / 31!
};

static f64 SineRadiansC_MFTWP[][11] =
{
    // NOTE(casey): This minimax coefficient table was donated by Demetri Spanos

    {},
    {},
    {0x1.fc4eac57b4a27p-1, -0x1.2b704cf682899p-3},
    {0x1.fff1d21fa9dedp-1, -0x1.53e2e5c7dd831p-3, 0x1.f2438d36d9dbbp-8},
    {0x1.ffffe07d31fe8p-1, -0x1.554f800fc5ea1p-3, 0x1.105d44e6222ap-7, -0x1.83b9725dff6e8p-13},
    {0x1.ffffffd25a681p-1, -0x1.555547ef5150bp-3, 0x1.110e7b396c557p-7, -0x1.9f6445023f795p-13, 0x1.5d38b56aee7f1p-19},
    {0x1.ffffffffd17d1p-1, -0x1.55555541759fap-3, 0x1.11110b74adb14p-7, -0x1.a017a8fe15033p-13, 0x1.716ba4fe56f6ep-19, -0x1.9a0e192a4e2cbp-26},
    {0x1.ffffffffffdcep-1, -0x1.5555555540b9bp-3, 0x1.111111090f0bcp-7, -0x1.a019fce979937p-13, 0x1.71dce5ace58d2p-19, -0x1.ae00fd733fe8dp-26, 0x1.52ace959bd023p-33},
    {0x1.fffffffffffffp-1, -0x1.5555555555469p-3, 0x1.111111110941dp-7, -0x1.a01a0199e0eb3p-13, 0x1.71de37e62aacap-19, -0x1.ae634d22bb47cp-26, 0x1.60e59ae00e00cp-33, -0x1.9ef5d594b342p-41},
    {0x1p0, -0x1.5555555555555p-3, 0x1.11111111110c9p-7, -0x1.a01a01a014eb6p-13, 0x1.71de3a52aab96p-19, -0x1.ae6454d960ac4p-26, 0x1.6123ce513b09fp-33, -0x1.ae43dc9bf8ba7p-41, 0x1.883c1c5deffbep-49},
    {0x1p0, -0x1.5555555555555p-3, 0x1.11111111110dcp-7, -0x1.a01a01a016ef6p-13, 0x1.71de3a53fa85cp-19, -0x1.ae6455b871494p-26, 0x1.612421756f93fp-33, -0x1.ae671378c3d43p-41, 0x1.90277dafc8ab9p-49, -0x1.78262e1f2709cp-58},
    {0x1p0, -0x1.5555555555555p-3, 0x1.11111111110dp-7, -0x1.a01a01a01559ap-13, 0x1.71de3a52ad36dp-19, -0x1.ae64549aa7ca9p-26, 0x1.612392f66fdcdp-33, -0x1.ae11556cad6c4p-41, 0x1.71744c339ad03p-49, 0x1.52947c90f8199p-55, -0x1.ff1898c107cfap-59},
};


static inline f64 Square(f64 X) {
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
  if (X > PiOverTwo) {
    X -= 2 * Pi64;
  }
  return SinCub(X + PiOverTwo);
}

static inline f64 Factorial(u64 X) {
  f64 Result = (f64)X;
  while (X > 1) {
    Result *= --X;
  }
  return Result;
}

static inline f64 TaylorH(f64 X) {
  const u64 max_exp = 19;
  s64 sign = (max_exp + 3) % 4 ? -1 : 1;
  f64 XSquared = X * X;
  f64 res = 0;
  for (s32 cur_power = max_exp; cur_power > 0; cur_power-=2) {
    f64 coeff = (f64)sign / (f64)Factorial(cur_power);
    sign *= -1;
    res = res * XSquared + coeff;
  }
  res *= X;
  return res;
}

static inline f64 TaylorHfma(f64 X) {
  const u64 max_exp = 21;
  s64 sign = (max_exp + 3) % 4 ? -1 : 1;
  f64 XSquared = X * X;
  __m128d X2_m = _mm_set_sd(XSquared);
  __m128d res_m = _mm_set_sd(0);
  for (s32 cur_power = max_exp; cur_power > 0; cur_power-=2) {
    f64 coeff = (f64)sign / (f64)Factorial(cur_power);
    sign *= -1;
    __m128d coeff_m = _mm_set_sd(coeff);
    res_m = _mm_fmadd_sd(X2_m, res_m, coeff_m);
    // res = std::fma(res, XSquared, coeff);
  }
  f64 res = _mm_cvtsd_f64(res_m);
  res *= X;
  return res;
}

static inline f64 TaylorArray(f64 X, u64 max_power) {
  max_power = max_power / 2 + 1;
  f64 res = SineRadiansC_Taylor[max_power];
  f64 x2 = X * X;
  for (s64 cur_power = max_power - 1; cur_power >= 0; cur_power--) {
    res = fma(x2, res, SineRadiansC_Taylor[cur_power]);
  }
  res *= X;
  return res;
}

static inline f64 SinDimitri(f64 X, u64 coeff_count) {
  f64 *coeffs = SineRadiansC_MFTWP[coeff_count];
  f64 res = coeffs[coeff_count];
  f64 x2 = X * X;
  for (s64 cur_coeff = coeff_count - 1; cur_coeff >= 0; cur_coeff--) {
    res = fma(x2, res, coeffs[cur_coeff]);
  }
  res *= X;
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

static inline f64 AsinTaylorInner(f64 X, u64 max_pow) {
  f64 x2 = X * X;
  f64 res = ArcsineRadiansC_Taylor[--max_pow];
  while (max_pow) {
    res = fma(res, x2, ArcsineRadiansC_Taylor[--max_pow]);
  }
  res *= X;
  return res;
}

static inline f64 AsinTaylor(f64 X, u64 max_pow) {
  f64 res;
  if (X > 1/sqrt(2)) {
    f64 NewX = sqrt(1 - X * X);
    res = PiOverTwo - AsinTaylorInner(NewX, max_pow);
  } else {
    res = AsinTaylorInner(X, max_pow);
  }
  return res;
}

static inline f64 AsinMinMaxInner(f64 X, u64 max_coeff, f64 *table) {
  f64 res = table[--max_coeff];
  f64 x2 = X * X;
  while (max_coeff) {
    res = fma(res, x2, table[--max_coeff]);
  }
  res *= X;
  return res;
}

static inline f64 AsinMinMax(f64 X, u64 max_coeff, f64 *table) {
  f64 res;
  if (X > 1/sqrt(2)) {
    f64 NewX = sqrt(1 - X * X);
    res = PiOverTwo - AsinMinMaxInner(NewX, max_coeff, table);
  } else {
    res = AsinMinMaxInner(X, max_coeff, table);
  }
  return res;
}

int main(void) {
  math_tester Tester = {};

  printf("RANGE: [%+.24f, %+.24f]\n", 0.0, PiOverTwo);

#if 0
  while (PrecisionTest(&Tester, 0.0, PiOverTwo)) {
    f64 RefOutput = sin(Tester.InputValue);
    // TestResult(&Tester, RefOutput, SinQ(Tester.InputValue), "SinQ");
    // TestResult(&Tester, RefOutput, SinCub(Tester.InputValue), "SinCub");
    // TestResult(&Tester, RefOutput, Sin3(Tester.InputValue), "Sin3");
    // TestResult(&Tester, RefOutput, Sin5(Tester.InputValue), "Sin5");
    // TestResult(&Tester, RefOutput, Sin7(Tester.InputValue), "Sin7");
    // TestResult(&Tester, RefOutput, Sin9(Tester.InputValue), "Sin9");
    // TestResult(&Tester, RefOutput, Sin11(Tester.InputValue), "Sin11");
    // TestResult(&Tester, RefOutput, Sin13(Tester.InputValue), "Sin13");
    // TestResult(&Tester, RefOutput, Sin15(Tester.InputValue), "Sin15");
    // TestResult(&Tester, RefOutput, Sin17(Tester.InputValue), "Sin17");
    // TestResult(&Tester, RefOutput, Sin19(Tester.InputValue), "Sin19");

    // TestResult(&Tester, RefOutput, SinH3(Tester.InputValue), "SinH3");
    // TestResult(&Tester, RefOutput, SinH5(Tester.InputValue), "SinH5");
    // TestResult(&Tester, RefOutput, SinH7(Tester.InputValue), "SinH7");
    // TestResult(&Tester, RefOutput, SinH9(Tester.InputValue), "SinH9");
    // TestResult(&Tester, RefOutput, SinH11(Tester.InputValue), "SinH11");
    // TestResult(&Tester, RefOutput, SinH13(Tester.InputValue), "SinH13");
    // TestResult(&Tester, RefOutput, SinH15(Tester.InputValue), "SinH15");
    // TestResult(&Tester, RefOutput, SinH17(Tester.InputValue), "SinH17");
    TestResult(&Tester, RefOutput, SinH19(Tester.InputValue), "SinH19");

    // TestResult(&Tester, RefOutput, TaylorH(Tester.InputValue), "TaylorH");
    TestResult(&Tester, RefOutput, TaylorHfma(Tester.InputValue), "TaylorHfma");
    // TestResult(&Tester, RefOutput, TaylorArray(Tester.InputValue, 5), "TaylorArray[5]");
    // TestResult(&Tester, RefOutput, TaylorArray(Tester.InputValue, 13), "TaylorArray[13]");
    TestResult(&Tester, RefOutput, TaylorArray(Tester.InputValue, 19), "TaylorArray[19]");
    // TestResult(&Tester, RefOutput, TaylorArray(Tester.InputValue, 23), "TaylorArray[23]");
    // TestResult(&Tester, RefOutput, SinDimitri(Tester.InputValue, 5), "SinDimitri[5]");
    // TestResult(&Tester, RefOutput, SinDimitri(Tester.InputValue, 8), "SinDimitri[8]");
    // TestResult(&Tester, RefOutput, SinDimitri(Tester.InputValue, 9), "SinDimitri[9]");
    // TestResult(&Tester, RefOutput, SinDimitri(Tester.InputValue, 10), "SinDimitri[10]");
    TestResult(&Tester, RefOutput, SinDimitri(Tester.InputValue, 11), "SinDimitri[11]");

    TestResult(&Tester, RefOutput, SinFin(Tester.InputValue), "SinFin");
  }
#endif

  printf("RANGE: [%+.24f, %+.24f]\n", 0.0, 1.0);

  while (PrecisionTest(&Tester, 0.0, 1)) {
    f64 RefOutput = asin(Tester.InputValue);
    for (u64 i = 25; i < ArrayCount(ArcsineRadiansC_Taylor); i++) {
      TestResult(&Tester, RefOutput, AsinTaylor(Tester.InputValue, i), "AsinTaylor[%u]", i);
    }
    for (u64 i = 20; i < ArrayCount(ArcsineRadiansC_MFTWP); i++) {
      TestResult(&Tester, RefOutput, AsinMinMax(Tester.InputValue, i, ArcsineRadiansC_MFTWP[i]), "AsinMinMax[%u]", i);
    }
  }
  PrintResults(&Tester);
  printf("\n");

  return 0;
}
