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
  int naive_dial = 50;
  unsigned int naive_zeros = 0;
  unsigned int naive_passes = 0;
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
    const int naive_before = naive_dial;
    if (d == 'L') {
      for (int i = 0; i < n; i++) {
        naive_dial = (naive_dial + 99) % 100;
        if (naive_dial == 0) naive_passes++;
      }
      if (naive_dial == 0) naive_zeros++;
      if (r <= dial) {
        dial -= r;
        if (dial == 0) passes++;
      } else {
        if (dial != 0) passes++;
        dial -= r - 100;
      }
    } else {
      for (int i = 0; i < n; i++) {
        naive_dial = (naive_dial + 1) % 100;
        if (naive_dial == 0) naive_passes++;
      }
      if (naive_dial == 0) naive_zeros++;
      if (r < 100 - dial) {
        dial += r;
      } else {
        if (dial != 0) passes++;
        dial += r - 100;
      }
    }
    const int naive_after = naive_dial;
    assert(0 <= dial && dial < 100);
    if (dial == 0) zeros++;
    assert(naive_dial == dial);
    if (naive_passes != passes) {
      print("%c%d %d -> %d: naive=%d, smort=%d\n", d, n, naive_before, naive_after, naive_passes, passes);
      exit(1);
    }
    if (naive_zeros != zeros) {
      print("%c%d %d -> %d: naive_zeros=%d, zeros=%d\n", d, n, naive_before, naive_after, naive_zeros, zeros);
      exit(1);
    }
    assert(naive_zeros == zeros);
  }
  print("%d\n%d\n", zeros, passes);
}

// wrong: 5829
