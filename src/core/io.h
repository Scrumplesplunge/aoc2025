#pragma once

#include <stdarg.h>

#include "strlen.h"

void die(const char*);

char* format_uint(char* restrict out, unsigned x) {
  char buffer[10];
  int i = 10;
  do {
    --i;
    buffer[i] = '0' + (x % 10);
    x /= 10;
  } while (x);
  while (i < 10) *out++ = buffer[i++];
  return out;
}

char* format_ulong(char* restrict out, unsigned long long x) {
  char buffer[24];
  int i = 24;
  do {
    --i;
    buffer[i] = '0' + (x % 10);
    x /= 10;
  } while (x);
  while (i < 24) *out++ = buffer[i++];
  return out;
}

char* format_string(char* restrict out, const char* restrict s) {
  while (*s) *out++ = *s++;
  return out;
}

char* vformat(char* restrict out, const char* restrict pattern, va_list args) {
  while (pattern) {
    while (*pattern != '\0' && *pattern != '%') *out++ = *pattern++;
    if (*pattern++ == '\0') break;
    switch (*pattern++) {
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
        out = format_uint(out, va_arg(args, unsigned long long));
        break;
      case 'l':
        if (*pattern++ != 'l') die("bad format");
        switch (*pattern++) {
          case 'd': {
            const long long value = va_arg(args, long long);
            if (value < 0) {
              *out++ = '-';
              out = format_ulong(out, -value);
            } else {
              out = format_ulong(out, value);
            }
            break;
          }
          case 'u':
            out = format_ulong(out, va_arg(args, unsigned long long));
            break;
          default:
            die("bad format");
        }
        break;
      case 's':
        out = format_string(out, va_arg(args, const char*));
        break;
      default:
        die("bad format");
    }
  }
  return out;
}

__attribute__((format(printf, 2, 3))) char* format(char* restrict out,
                                                   const char* restrict pattern,
                                                   ...) {
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

void print_uint(unsigned int a) {
  char buffer[128];
  char* o = buffer;
  o = format_uint(o, a);
  *o++ = '\n';
  write(STDOUT_FILENO, buffer, o - buffer);
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

void print_ulongs(unsigned long long a, unsigned long long b) {
  char buffer[128];
  char* o = buffer;
  o = format_ulong(o, a);
  *o++ = '\n';
  o = format_ulong(o, b);
  *o++ = '\n';
  write(STDOUT_FILENO, buffer, o - buffer);
}

__attribute__((format(printf, 1, 2))) void print(const char* restrict pattern,
                                                 ...) {
  va_list args;
  va_start(args, pattern);
  vfprint(STDOUT_FILENO, pattern, args);
  va_end(args);
}

[[noreturn]] void die(const char* message) {
  write(STDERR_FILENO, message, strlen(message));
  exit(1);
}

bool is_digit(char c) { return '0' <= c && c <= '9'; }

const char* scan_uint(const char* restrict in, unsigned int* x) {
  *x = 0;
  if (!is_digit(*in)) return NULL;
  unsigned int value = 0;
  while (is_digit(*in)) value = 10 * value + (*in++ - '0');
  *x = value;
  return in;
}

const char* scan_ulong(const char* restrict in, unsigned long long* x) {
  *x = 0;
  if (!is_digit(*in)) return NULL;
  unsigned long long value = 0;
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
    if (*pattern++ == '\0') break;
    switch (*pattern++) {
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
      case 'l':
        if (*pattern++ != 'l') die("bad format");
        switch (*pattern++) {
          case 'd': {
            if (*in == '-') {
              unsigned long long x;
              in = scan_ulong(in, &x);
              if (!in) return NULL;
              *va_arg(args, long long*) = -x;
            } else {
              unsigned long long x;
              in = scan_ulong(in, &x);
              if (!in) return NULL;
              *va_arg(args, long long*) = x;
            }
            break;
          }
          case 'u':
            in = scan_ulong(in, va_arg(args, unsigned long long*));
            if (!in) return NULL;
            break;
          default:
            die("bad format");
        }
        break;
      default:
        die("bad format");
    }
    pattern += 2;
  }
  return in;
}

__attribute__((format(scanf, 2, 3))) const char* scan(
    const char* restrict in, const char* restrict pattern, ...) {
  va_list args;
  va_start(args, pattern);
  in = vscan(in, pattern, args);
  va_end(args);
  return in;
}
