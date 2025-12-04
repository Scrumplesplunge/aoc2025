#include "core/assert.h"
#include "core/io.h"

enum { max_size = 142 };
bool grid[max_size + 2][max_size + 2];

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

  // Solve part 1.
  unsigned part1 = 0;
  for (int y = 1; y <= height; y++) {
    for (int x = 1; x <= width; x++) {
      if (!grid[y][x]) continue;
      int adjacent = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0) continue;
          if (grid[y + dy][x + dx]) adjacent++;
        }
      }
      if (adjacent < 4) part1++;
    }
  }

  print_uints(part1, 0);
}
