#include "core/assert.h"
#include "core/io.h"

// Reserve some space for storing working grids. The normal input size is
// 140x140 and we additionally reserve a margin of empty cells around the edge
// so that we don't need to check for boundary conditions when looking for
// adjacent cells. This cuts 23% off the execution time.
enum { max_size = 140 };
bool grid[max_size + 2][max_size + 2];
struct pos { unsigned char x, y; };

enum { max_stack_size = max_size * max_size };
struct pos stack[max_size * max_size];

int adjacent(struct pos pos) {
  int result = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) continue;
      if (grid[pos.y + dy][pos.x + dx]) result++;
    }
  }
  return result;
}

int main() {
  char buffer[20 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
  }

  // Determine the grid dimensions.
  int width = 0;
  while (buffer[width] != '\n') width++;
  const int height = length / (width + 1);
  if (length % (width + 1) != 0) die("bad input");

  // Copy the grid into something easier to work with.
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const char cell = buffer[y * (width + 1) + x];
      if (cell != '.' && cell != '@') die("bad input");
      grid[y + 1][x + 1] = (cell == '@');
    }
  }

  // Part 1: Identify all the removable nodes.
  unsigned stack_size = 0;
  for (int y = 1; y <= height; y++) {
    for (int x = 1; x <= width; x++) {
      if (!grid[y][x]) continue;
      const struct pos pos = {x, y};
      if (adjacent(pos) < 4) stack[stack_size++] = pos;
    }
  }
  const unsigned part1 = stack_size;

  // Part 2: Iteratively remove nodes.
  unsigned part2 = 0;
  while (stack_size > 0) {
    struct pos pos = stack[--stack_size];
    if (!grid[pos.y][pos.x] || adjacent(pos) >= 4) continue;
    part2++;
    grid[pos.y][pos.x] = false;
    for (int dy = -1; dy <= 1; dy++) {
      for (int dx = -1; dx <= 1; dx++) {
        const struct pos neighbor = {pos.x + dx, pos.y + dy};
        if (grid[neighbor.y][neighbor.x]) stack[stack_size++] = neighbor;
      }
    }
  }

  print_uints(part1, part2);
}
