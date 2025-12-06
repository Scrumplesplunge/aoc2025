#include "core/assert.h"
#include "core/io.h"

typedef unsigned int uint32;
typedef unsigned long long uint64;

enum { buffer_size = 20 << 10 };
char buffer[buffer_size];
enum { max_lines = 8 };
int num_lines;
char* lines[max_lines];

void read_input() {
  const int length = read(STDIN_FILENO, buffer, buffer_size);
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
  }
  char* i = buffer;
  char* const end = buffer + length;
  while (i != end) {
    if (num_lines == max_lines) die("bad input");
    lines[num_lines++] = i;
    while (*i != '\n') i++;
    i++;
  }
  if (num_lines < 2) die("bad input");
}

struct op {
  uint64 initial;
  uint64 (*apply)(uint64 a, uint64 b);
};

uint64 add(uint64 a, uint64 b) { return a + b; }
uint64 mul(uint64 a, uint64 b) { return a * b; }

const struct op op_add = {.initial = 0, .apply = add};
const struct op op_mul = {.initial = 1, .apply = mul};

struct op get_op(char op) {
  switch (op) {
    case '+':
      return op_add;
    case '*':
      return op_mul;
  }
  die("bad input");
}

uint64 part1() {
  const int n = num_lines;
  char* is[max_lines];
  for (int i = 0; i < n; i++) is[i] = lines[i];
  uint64 total = 0;
  while (true) {
    // Strip leading whitespace.
    for (int i = 0; i < n; i++) {
      while (*is[i] == ' ') is[i]++;
    }
    const char opcode = *is[num_lines - 1]++;
    if (opcode == '\n') break;
    const struct op op = get_op(opcode);
    uint64 value = op.initial;
    for (int i = 0; i < num_lines - 1; i++) {
      uint32 x;
      is[i] = scan_uint(is[i], &x);
      if (!is[i]) die("bad input");
      value = op.apply(value, x);
    }
    total += value;
  }
  return total;
}

uint64 part2() {
  const int line_length = lines[1] - lines[0] - 1;

  enum { max_values = 4 };
  int num_values = 0;
  uint64 values[max_values];

  uint64 total = 0;
  const char* ops = lines[num_lines - 1];
  for (int i = line_length - 1; i >= 0; i--) {
    // Parse the value from this column.
    uint64 x = 0;
    int digits = 0;
    for (int j = 0; j < num_lines - 1; j++) {
      const char c = lines[j][i];
      if (c != ' ') {
        digits++;
        assert(is_digit(c));
        x = 10 * x + (lines[j][i] - '0');
      }
    }
    if (!digits) continue;
    // Add the value to the list for this problem.
    print("%llu\n", x);
    if (num_values == max_values) die("bad input nv");
    values[num_values++] = x;
    // Check if this is the end of a single problem.
    if (ops[i] != ' ') {
      const struct op op = get_op(ops[i]);
      uint64 value = op.initial;
      for (int i = 0; i < num_values; i++) {
        value = op.apply(value, values[i]);
      }
      print("%c -> %llu\n", ops[i], value);
      total += value;
      num_values = 0;
    }
  }
  assert(num_values == 0);
  return total;
}

int main() {
  read_input();
  print_ulongs(part1(), part2());
}

// 14270832890392589 too high
