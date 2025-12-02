#pragma once

#include "assert.h"

// GCC generates calls to this function even with -nostdlib -nostartfiles
// so we need to provide one.
unsigned long long __udivmoddi4(unsigned long long n, unsigned long long d,
                                unsigned long long* r) {
  unsigned long long q = 0, x = 1;
  assert(d != 0);
  while ((long long)d >= 0) {
    d <<= 1;
    x <<= 1;
  }
  while (x) {
    if (d <= n) {
      n -= d;
      q += x;
    }
    d >>= 1;
    x >>= 1;
  }
  if (r) *r = n;
  return q;
}
