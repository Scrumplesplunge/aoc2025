#pragma once

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
  char* o = dest;
  char* const end = o + n;
  const char* i = src;
  while (o != end) *o++ = *i++;
  return dest;
}
