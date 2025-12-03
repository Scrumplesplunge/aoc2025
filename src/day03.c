// Finding the largest number formed from a subsequence of n values.
//
//   Part 1: 2 values.
//   Part 2: 12 values.
//
// We can do this by maintaining a current best value and updating it as we scan
// the input. Here, I chose to start with the last n values of a line and update
// the list for each preceding character.

#include "core/assert.h"
#include "core/io.h"

unsigned long long joltage(const char* first, const char* last, int n) {
  assert(n <= 12);
  char buffer[13] = {};
  char* buffer_end = buffer + n;
  *buffer_end = '\0';
  memcpy(buffer, last - n, n);
  for (const char* i = last - n; i != first; i--) {
    const char c = i[-1];
    // If the digit is smaller than the leading digit of our current best, it's
    // useless to us: including this digit will make the value smaller.
    if (c < buffer[0]) continue;
    // Otherwise, we find the longest descending prefix and drop the last digit
    // from it. This works because we've effectively made every digit of our
    // subsequence larger up until the end of that prefix, and left every digit
    // the same after that.
    char temp = c;
    for (char* j = buffer; j != buffer_end; j++) {
      if (*j > temp) break;
      char x = *j;
      *j = temp;
      temp = x;
    }
  }
  unsigned long long x;
  const char* result = scan_ulong(buffer, &x);
  assert(result == buffer_end);
  return x;
}

int main() {
  char buffer[21 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
  }
  const char* i = buffer;
  const char* const end = buffer + length;

  unsigned long long part1 = 0;
  unsigned long long part2 = 0;
  while (i != end) {
    const char* first = i;
    while (*i != '\n') i++;
    const char* last = i;
    *(char*)i = '\0';
    i++;

    part1 += joltage(first, last, 2);
    part2 += joltage(first, last, 12);
  }

  print_ulongs(part1, part2);
}
