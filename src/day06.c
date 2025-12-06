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

int main() {
  read_input();

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
      print("attempt to scan line %d at %c\n", i, *is[i]);
      is[i] = scan_uint(is[i], &x);
      if (!is[i]) die("bad input");
      value = op.apply(value, x);
    }
    print("column %c result: %llu\n", *is[num_lines - 1], value);
    total += value;
  }

  print_ulongs(total, 0);
}

// 14270832890392589 too high
