#include "core/assert.h"
#include "core/io.h"

struct inclusive_range {
  unsigned long long first, last;
};

enum { max_ranges = 200, max_ingredients = 1000 };

// `ranges` is a list of non-overlapping, non-consecutive (so the end of one
// range is not consecutive with the start of the next) ranges of fresh
// ingredients.
int num_ranges;
struct inclusive_range ranges[max_ranges];
int num_ingredients;
unsigned long long ingredients[max_ingredients];

void add_fresh_range(struct inclusive_range range) {
  // Find the range of ranges which overlap or concatenate with `range`.
  const int n = num_ranges;

  int j = 0;
  for (int i = 0; i < n; i++) {
    struct inclusive_range* r = &ranges[i];
    if (range.last + 1 >= r->first && r->last + 1 >= range.first) {
      if (r->first < range.first) range.first = r->first;
      if (r->last > range.last) range.last = r->last;
    } else {
      ranges[j++] = *r;
    }
  }
  if (j == max_ranges) die("too large");
  num_ranges = j + 1;
  while (j > 0 && range.last < ranges[j - 1].first) {
    ranges[j] = ranges[j - 1];
    j--;
  }
  ranges[j] = range;
}

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
    struct inclusive_range range;
    i = scan_ulong(i, &range.first);
    if (!i || *i++ != '-') die("bad input");
    i = scan_ulong(i, &range.last);
    if (!i || *i++ != '\n') die("bad input");
    add_fresh_range(range);
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
  const struct inclusive_range* i = ranges;
  int length = num_ranges;
  while (length > 0) {
    const int r = length % 2;
    length /= 2;
    i += (i[length].last < id) * (length + r);
  }
  return i->first <= id && id <= i->last;
}

int main() {
  read_input();

  const int n = num_ingredients;
  int part1 = 0;
  for (int i = 0; i < n; i++) {
    if (is_fresh(ingredients[i])) part1++;
  }

  unsigned long long part2 = 0;
  const int r = num_ranges;
  for (int i = 0; i < r; i++) {
    part2 += ranges[i].last - ranges[i].first + 1;
  }

  print_ulongs(part1, part2);
}
