#include "core/assert.h"
#include "core/io.h"

struct inclusive_range {
  unsigned long long first, last;
};

enum { max_ranges = 200, max_ingredients = 1000 };
int num_ranges;
struct inclusive_range ranges[max_ranges];
int num_ingredients;
unsigned long long ingredients[max_ingredients];

void read_input() {
  char buffer[22 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
  }
  const char* i = buffer;
  const char* const end = buffer + length;
  // Scan the fresh ranges.
  while (*i != '\n') {
    if (num_ranges == max_ranges) die("too large");
    struct inclusive_range* range = &ranges[num_ranges++];
    i = scan_ulong(i, &range->first);
    if (!i || *i++ != '-') die("bad input");
    i = scan_ulong(i, &range->last);
    if (!i || *i++ != '\n') die("bad input");
  }
  // Scan the ingredient IDs.
  i++;
  while (i != end) {
    if (num_ingredients == max_ingredients) die("too large");
    i = scan_ulong(i, &ingredients[num_ingredients++]);
    if (!i || *i++ != '\n') die("bad input");
  }
}

bool is_fresh(unsigned long long id) {
  const int n = num_ranges;
  for (int i = 0; i < n; i++) {
    struct inclusive_range range = ranges[i];
    if (range.first <= id && id <= range.last) return true;
  }
  return false;
}

int main() {
  read_input();

  const int n = num_ingredients;
  int part1 = 0;
  for (int i = 0; i < n; i++) {
    if (is_fresh(ingredients[i])) part1++;
  }

  print_uints(part1, 0);
}
