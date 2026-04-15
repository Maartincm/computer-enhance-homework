#pragma once
#include "types.h"
#include <math.h>

inline f64 Power(f64 X, u64 E) {
  return pow(X, E);
  // f64 res = X;
  // for (u64 i = 0; i < E - 1; i++) {
  //   res *= X;
  // }
  // return res;
}
