#pragma once

#include <stdarg.h>

void die(const char* restrict pattern, ...);

char* format_uint(char* restrict out, unsigned int x) {
  char buffer[16];
  int i = 16;
  do {
    --i;
    buffer[i] = '0' + (x % 10);
    x /= 10;
  } while (x);
  while (i < 16) *out++ = buffer[i++];
  return out;
}

char* format_string(char* restrict out, const char* restrict s) {
  while (*s) *out++ = *s++;
  return out;
}

char* vformat(char* restrict out, const char* restrict pattern, va_list args) {
  while (pattern) {
    while (*pattern != '\0' && *pattern != '%') *out++ = *pattern++;
    if (*pattern == '\0') break;
    switch (pattern[1]) {
      case '%':
        *out++ = '%';
        break;
      case 'c':
        *out++ = va_arg(args, int);
        break;
      case 'd': {
        const int value = va_arg(args, int);
        if (value < 0) {
          *out++ = '-';
          out = format_uint(out, -value);
        } else {
          out = format_uint(out, value);
        }
        break;
      }
      case 'u':
        out = format_uint(out, va_arg(args, unsigned int));
        break;
      case 's':
        out = format_string(out, va_arg(args, const char*));
        break;
      default:
        die("bad format %%%c\n", pattern[1]);
    }
    pattern += 2;
  }
  return out;
}

char* format(char* restrict out, const char* restrict pattern, ...) {
  va_list args;
  va_start(args, pattern);
  out = vformat(out, pattern, args);
  va_end(args);
  return out;
}

void vfprint(int fd, const char* restrict pattern, va_list args) {
  char buffer[128];
  char* end = vformat(buffer, pattern, args);
  write(fd, buffer, end - buffer);
}

void fprint(int fd, const char* restrict pattern, ...) {
  va_list args;
  va_start(args, pattern);
  vfprint(fd, pattern, args);
  va_end(args);
}

void print_uints(unsigned int a, unsigned int b) {
  char buffer[128];
  char* o = buffer;
  o = format_uint(o, a);
  *o++ = '\n';
  o = format_uint(o, b);
  *o++ = '\n';
  write(STDOUT_FILENO, buffer, o - buffer);
}

void print(const char* restrict pattern, ...) {
  va_list args;
  va_start(args, pattern);
  vfprint(STDOUT_FILENO, pattern, args);
  va_end(args);
}

void die(const char* restrict pattern, ...) {
  va_list args;
  va_start(args, pattern);
  vfprint(STDERR_FILENO, pattern, args);
  va_end(args);
  exit(1);
}

bool is_digit(char c) { return '0' <= c && c <= '9'; }

const char* scan_uint(const char* restrict in, unsigned int* x) {
  if (!is_digit(*in)) return NULL;
  unsigned int value = 0;
  while (is_digit(*in)) value = 10 * value + (*in++ - '0');
  *x = value;
  return in;
}

const char* vscan(const char* restrict in, const char* restrict pattern,
                  va_list args) {
  while (pattern) {
    while (*pattern != '\0' && *pattern != '%') {
      if (*in++ != *pattern++) return NULL;
    }
    if (*pattern == '\0') break;
    switch (pattern[1]) {
      case '%':
        if (*in++ != '%') return NULL;
        break;
      case 'c':
        *va_arg(args, char*) = *in++;
        break;
      case 'd': {
        if (*in == '-') {
          unsigned int x;
          in = scan_uint(in, &x);
          if (!in) return NULL;
          *va_arg(args, int*) = -x;
        } else {
          unsigned int x;
          in = scan_uint(in, &x);
          if (!in) return NULL;
          *va_arg(args, int*) = x;
        }
        break;
      }
      case 'u':
        in = scan_uint(in, va_arg(args, unsigned int*));
        if (!in) return NULL;
        break;
      default:
        die("bad format %%%c\n", pattern[1]);
    }
    pattern += 2;
  }
  return in;
}

const char* scan(const char* restrict in, const char* restrict pattern, ...) {
  va_list args;
  va_start(args, pattern);
  in = vscan(in, pattern, args);
  va_end(args);
  return in;
}
