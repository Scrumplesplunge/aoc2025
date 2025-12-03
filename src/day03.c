#include "core/io.h"

int main() {
  char buffer[21 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
  }
  const char* i = buffer;
  const char* const end = buffer + length;

  int sum = 0;
  while (i != end) {
    const char* first = i;
    while (*i != '\n') i++;
    const char* last = i;
    i++;

    char joltage[2] = {first[0], last[-1]};
    for (const char* j = first + 1; j + 1 != last; j++) {
      if (*j > joltage[0]) {
        joltage[0] = *j;
        joltage[1] = last[-1];
      } else if (*j > joltage[1]) {
        joltage[1] = *j;
      }
    }

    sum += 10 * (joltage[0] - '0') + (joltage[1] - '0');
  }

  print_uints(sum, 0);
}
