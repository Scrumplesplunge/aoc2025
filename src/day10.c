#include "core/io.h"

typedef unsigned short uint16;

enum { max_buttons = 14 };
struct machine {
  uint16 target;
  uint16 num_buttons;
  uint16 buttons[max_buttons];
};

enum { max_machines = 200 };
int num_machines;
struct machine machines[max_machines];

void read_input() {
  enum { buffer_size = 21 << 10 };
  char buffer[buffer_size];
  const int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input len");

  const char* i = buffer;
  const char* end = i + n;

  while (i != end) {
    if (num_machines == max_machines) die("bad input machine count");
    struct machine* const m = &machines[num_machines++];
    if (*i++ != '[') die("bad input");
    // Read the target light sequence.
    uint16 next_bit = 1;
    while (*i == '.' || *i == '#') {
      if (next_bit == (1 << max_buttons)) die("bad input mask size");
      if (*i == '#') m->target |= next_bit;
      next_bit <<= 1;
      i++;
    }
    if (*i++ != ']' || *i++ != ' ' || *i++ != '(') die("bad input syntax");
    // Read the set of buttons.
    while (true) {
      if (m->num_buttons == max_buttons) die("bad input button count");
      uint16* const button = &m->buttons[m->num_buttons++];
      while (true) {
        if (!is_digit(*i)) die("bad input syntax2");
        *button |= 1 << (*i++ - '0');
        if (*i != ',') break;
        i++;
      }
      if (*i++ != ')' || *i++ != ' ') die("bad input syntax3");
      if (*i != '(') break;
      i++;
    }
    if (*i++ != '{') die("bad input syntax4");
    // Skip the joltages for now.
    while (*i++ != '}') {}
    if (*i++ != '\n') die("bad input syntax5");
  }
}

int popcount(unsigned int x) {
  int total;
  asm("popcnt %1, %0" : "=r"(total) : "r"(x));
  return total;
}

int part1() {
  int total = 0;
  for (int i = 0, n = num_machines; i < n; i++) {
    const struct machine* m = &machines[i];
    int best_count = max_buttons + 1;
    for (int set = 0, all = (1 << m->num_buttons) - 1; set < all; set++) {
      uint16 result = 0;
      for (int i = 0; i < max_buttons; i++) {
        if (set & (1 << i)) result ^= m->buttons[i];
      }
      if (result != m->target) continue;
      const int count = popcount(set);
      if (count >= best_count) continue;
      best_count = count;
    }
    total += best_count;
  }
  return total;
}

int main() {
  read_input();
  print_uints(part1(), 0);
}
