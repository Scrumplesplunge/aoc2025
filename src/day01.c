#include "core/assert.h"
#include "core/io.h"

int main() {
  char buffer[20 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input l");
  }
  const char* i = buffer;
  const char* const end = buffer + length;
  int dial = 50;
  unsigned int zeros = 0;
  unsigned int passes = 0;
  while (i != end) {
    char d;
    int n;
    i = scan(i, "%c%d\n", &d, &n);
    assert(n > 0);
    if (!i || (d != 'L' && d != 'R')) die("bad input t");
    const int q = n / 100, r = n % 100;
    passes += q;
    if (d == 'L') {
      if (r <= dial) {
        dial -= r;
        if (dial == 0) passes++;
      } else {
        if (dial != 0) passes++;
        dial -= r - 100;
      }
    } else {
      if (r < 100 - dial) {
        dial += r;
      } else {
        if (dial != 0) passes++;
        dial += r - 100;
      }
    }
    if (dial == 0) zeros++;
  }
  print("%d\n%d\n", zeros, passes);
}

// wrong: 5829
