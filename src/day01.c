// Counting times where a dial hits zero:
//
//   Part 1: inbetween distinct moves.
//   Part 2: at any point during a move.
//
// This is a typical modular arithmetic puzzle, but there are several edge cases
// to consider, particularly regarding cases where a move starts or ends on 0.
// This solution simplifies the tracking of zero crossings by:
//
//   * Turning any 0 into a 100 at the end of a move.
//   * Focusing on left turns, and handling right turns by inverting the dial.
//     This flipped state is tracked by a parity field: 'L' means unflipped,
//     'R' means flipped.

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
  char parity = 'L';
  int dial = 50;
  unsigned int zeros = 0;
  unsigned int passes = 0;
  while (i != end) {
    if (*i != 'L' && *i != 'R') die("bad input");
    char d = *i++;
    unsigned int n;
    i = scan_uint(i, &n);
    if (i == NULL || *i != '\n') die("bad input");
    i++;
    const int q = n / 100, r = n % 100;
    passes += q;
    if (d != parity) {
      parity = d;
      dial = 100 - dial;
    }
    if (dial == 0 || dial == 100) {
      dial = 100 - r;
    } else {
      dial -= r;
      if (dial == 0) zeros++;
      if (dial <= 0) {
        passes++;
        dial += 100;
      }
    }
  }
  print_uints(zeros, passes);
}
