#pragma once

#include "io.h"

#ifndef NDEBUG
#define assert(expr) \
  if (!(expr)) die("%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #expr)
#else
#define assert(expr)
#endif
