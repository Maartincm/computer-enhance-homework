#include <cstdio>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include "types.h"

#include "listing_0175_math_check.cpp"

static u64 Factorial(u64 X) {
  f64 Result = X;
  for (u64 i = X - 1; i > 1; i--) {
    Result *= i;
  }
  return Result;
}

#define MAX_SERIES 19
int gen_horner(void) {
  FILE *gen_series_file = fopen("generated_series_horner.cpp", "w");
  fprintf(gen_series_file, "#include \"types.h\"\n");
  for (u32 i = 3; i <= MAX_SERIES; i+=2) {
    fprintf(gen_series_file, "static f64 SinH%uZeroToPiOverTwo(f64 X) {\n", i);
    s32 sign = 1;
    for (u32 cur_power = 1; cur_power <= i; cur_power+=2) {
      f64 coeff = sign * 1.0L / Factorial(cur_power);
      sign *= -1;
      fprintf(gen_series_file, "  f64 c%u = %.60f;\n", cur_power, coeff);
    }
    fprintf(gen_series_file, "  f64 XSquared = X * X;\n");
    fprintf(gen_series_file, "  f64 result = ");

    for (u32 phar = 0; phar < i/2; phar++) {
      fprintf(gen_series_file, "(");
    }
    fprintf(gen_series_file, "c%u * XSquared + c%u)", i, i - 2);
    for (s32 cur_power = i - 4; cur_power > 0; cur_power-=2) {
      fprintf(gen_series_file, " * XSquared + c%u)", cur_power);
    }
    fprintf(gen_series_file, " * X;\n");
    fprintf(gen_series_file, "  return result;\n");
    fprintf(gen_series_file, "}\n");
    fprintf(gen_series_file, "static f64 SinH%uPos(f64 X) {\n  if(X > PiOverTwo) {\n    X = Pi64 - X;\n  }\n  f64 res = SinH%uZeroToPiOverTwo(X);\n  return res;\n}\n", i, i);
    fprintf(gen_series_file, "static f64 SinH%u(f64 X) {\n  f64 res;\n  if(X < 0) {\n    res = -SinH%uPos(-X);\n  } else {\n    res = SinH%uPos(X);\n  }\n  return res;\n}\n\n", i, i, i);
  }
  fclose(gen_series_file);
  return 0;
}

int gen_taylor(void) {
  FILE *gen_series_file = fopen("generated_series_taylor.cpp", "w");
  fprintf(gen_series_file, "#include \"types.h\"\n");
  fprintf(gen_series_file, "#include \"power.h\"\n\n");
  for (u32 i = 3; i <= MAX_SERIES; i+=2) {
    fprintf(gen_series_file, "static f64 Sin%uZeroToPiOverTwo(f64 X) {\n", i);
    s32 sign = 1;
    for (u32 cur_power = 1; cur_power <= i; cur_power+=2) {
      f64 coeff = sign * 1.0L / Factorial(cur_power);
      sign *= -1;
      fprintf(gen_series_file, "  f64 c%u = %.60f;\n", cur_power, coeff);
    }
    fprintf(gen_series_file, "  f64 result = ");
    for (u32 cur_power = 1; cur_power <= i; cur_power+=2) {
      fprintf(gen_series_file, "c%u * Power(X, %u) + ", cur_power, cur_power);
    }
    fseek(gen_series_file, -3, SEEK_CUR);
    fprintf(gen_series_file, ";\n");
    fprintf(gen_series_file, "  return result;\n");
    fprintf(gen_series_file, "}\n");
    fprintf(gen_series_file, "static f64 Sin%uPos(f64 X) {\n  if(X > PiOverTwo) {\n    X = Pi64 - X;\n  }\n  f64 res = Sin%uZeroToPiOverTwo(X);\n  return res;\n}\n", i, i);
    fprintf(gen_series_file, "static f64 Sin%u(f64 X) {\n  f64 res;\n  if(X < 0) {\n    res = -Sin%uPos(-X);\n  } else {\n    res = Sin%uPos(X);\n  }\n  return res;\n}\n\n", i, i, i);
  }
  fclose(gen_series_file);
  return 0;
}

int main(void) {
  gen_taylor();
  gen_horner();
}
