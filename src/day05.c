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
  int f = 0;
  while (f < n && ranges[f].last + 1 < range.first) f++;
  int l = f;
  while (l < n && ranges[l].first < range.last + 1) l++;

  if (f == n) {
    // Strictly after all existing ranges.
    if (num_ranges == max_ranges) die("too large");
    ranges[num_ranges++] = range;
  } else if (l == n) {
    // Range is last, but overlaps with some.
    ranges[f].last = range.last;
    num_ranges = f + 1;
  } else if (f == l) {
    // Range inserts at f.
    if (num_ranges == max_ranges) die("too large");
    memmove(ranges + f + 1, ranges + f,
            (n - f) * sizeof(struct inclusive_range));
    ranges[f] = range;
    num_ranges++;
  } else {
    // Range overlaps or concatenates with some entries.
    if (ranges[f].first < range.first) range.first = ranges[f].first;
    if (ranges[l - 1].last > range.last) range.last = ranges[l - 1].last;
    memmove(ranges + f + 1, ranges + l,
            (n - l) * sizeof(struct inclusive_range));
    ranges[f] = range;
    num_ranges -= l - f - 1;
  }
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

  unsigned long long part2 = 0;
  const int r = num_ranges;
  for (int i = 0; i < r; i++) {
    part2 += ranges[i].last - ranges[i].first + 1;
  }

  print_ulongs(part1, part2);
}
