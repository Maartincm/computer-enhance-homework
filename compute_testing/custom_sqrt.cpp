#include <cmath>
#include <cstdio>
#include <xmmintrin.h>

#include "types.h"

typedef f64 (SqrtFunc)(f64);

typedef struct TestFunc_ {
    const char *funcName;
    SqrtFunc *func;
} TestFunc;

f64 custom_sqrt(f64 num) {
    __m128d in = _mm_set_sd(num);
    // __m128d mask = _mm_set_sd(0);
    __m128d sqrt_res;
    sqrt_res = _mm_sqrt_sd(sqrt_res, in);
    f64 res = _mm_cvtsd_f64(sqrt_res);
    return res;
}

f64 custom_sqrt2(f64 num) {
    __m128 in = _mm_set_ss(num);
    // __m128d mask = _mm_set_sd(0);
    __m128 rsqrt_res;
    rsqrt_res = _mm_rsqrt_ss(in);
    float res = _mm_cvtss_f32(rsqrt_res);
    return 1.0 / res;
}

int main(int argc, char *argv[]) {
    f64 tests[] = {1.3234256, 5723.1923121, 4.4444};
    TestFunc funcArray[] = {{"sqrt", sqrt}, {"custom_sqrt", custom_sqrt}, {"custom_sqrt2", custom_sqrt2}};

    for (u32 i = 0; i < ArrayCount(tests); i++) {
        for (u32 funcIndex = 0; funcIndex < ArrayCount(funcArray); funcIndex++) {
            TestFunc tf = funcArray[funcIndex];
            f64 in = tests[i];
            f64 res = tf.func(in);
            printf("%s(%f) = %f\n", tf.funcName, in, res);
        }
    }

    return 0;
}
