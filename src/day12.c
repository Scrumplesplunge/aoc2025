#include "core/io.h"

typedef unsigned char uint8;

const char* read_num(const char* x, uint8* n) {
  if (!is_digit(x[0])) die("bad input");
  if (is_digit(x[1])) {
    *n = 10 * (x[0] - '0') + (x[1] - '0');
    return x + 2;
  } else {
    *n = (x[0] - '0');
    return x + 1;
  }
}

int main() {
  enum { buffer_size = 25 << 10 };
  char buffer[buffer_size];
  const int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input");

  const char* i = buffer;
  const char* const end = i + n;

  // Read the shapes.
  enum { num_shapes = 6 };
  int shapes[num_shapes];
  for (int s = 0; s < num_shapes; s++) {
    if (*i++ != ('0' + s) || *i++ != ':' || *i++ != '\n') die("bad input");
    int parts = 0;
    while (i[0] != '\n' || i[1] != '\n') {
      parts += (i[0] == '#');
      i++;
    }
    i += 2;
    shapes[s] = parts;
  }

  // Process the regions.
  int num_valid = 0;
  while (i != end) {
    uint8 width, height;
    i = read_num(i, &width);
    if (*i++ != 'x') die("bad input");
    i = read_num(i, &height);
    if (*i++ != ':') die("bad input");
    int required_area = 0;
    int required_slots = 0;
    for (int c = 0; c < num_shapes; c++) {
      if (*i++ != ' ') die("bad input");
      uint8 count;
      i = read_num(i, &count);
      required_area += shapes[c] * count;
      required_slots += count;
    }
    if (*i++ != '\n') die("bad input");

    // Obvious no if the total area of required shapes is larger than the
    // available area, disregarding shape.
    if (required_area > width * height) continue;

    // Obvious yes if we can fit all the shapes in a neat grid of 3x3 blocks
    // without attempting any kind of overlap.
    if (required_slots <= (width / 3) * (height / 3)) {
      num_valid++;
      continue;
    }

    // Otherwise, we need to try carefully packing shapes. That sounds hard.
    die("too hard");
  }
  print_uint(num_valid);
}
