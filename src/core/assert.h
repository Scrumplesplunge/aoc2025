#pragma once

#include "io.h"

[[noreturn]] void assert_fail(const char* restrict file, int line,
                              const char* restrict expr) {
  fprint(STDERR_FILENO, "%s:%d: assertion failed: %s\n", file, line, expr);
  exit(1);
}

#ifndef NDEBUG
#define assert(expr) \
  if (!(expr)) assert_fail(__FILE__, __LINE__, #expr)
#else
#define assert(expr)
#endif
