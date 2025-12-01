#include "core/assert.h"
#include "core/io.h"

int main() {
  char buffer[20 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
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
    if (!i || (d != 'L' && d != 'R')) die("bad input");
    // Tick the dial one step at a time. Use 99 instead of -1 to avoid
    // having to deal with negative values in the modulo in the loop.
    const int step = d == 'L' ? 99 : 1;
    for (int i = 0; i < n; i++) {
      dial = (dial + step) % 100;
      if (dial == 0) passes++;
    }
    if (dial == 0) zeros++;
  }
  print("%d\n%d\n", zeros, passes);
}
