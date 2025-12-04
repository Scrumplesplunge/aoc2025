#include "core/assert.h"
#include "core/io.h"

// Reserve some space for storing working grids. The normal input size is
// 140x140 and we additionally reserve a margin of empty cells around the edge
// so that we don't need to check for boundary conditions when looking for
// adjacent cells. This cuts 23% off the execution time.
enum { max_size = 140 };
bool grid[2][max_size + 2][max_size + 2];

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
      grid[0][y + 1][x + 1] = (cell == '@');
    }
  }

  int iteration = 0;
  unsigned part1 = 0;
  unsigned part2 = 0;
  while (true) {
    // In each iteration, calculate the rolls which are removed and update the
    // grid. To avoid changes influencing checks for rolls later in the same
    // iteration, each iteration reads from one grid and writes to another. The
    // `from` and `to` grids alternate in each iteration.
    const bool (*from)[max_size + 2] = grid[iteration % 2];
    bool (*to)[max_size + 2] = grid[(iteration + 1) % 2];

    unsigned num_removed = 0;
    for (int y = 1; y <= height; y++) {
      for (int x = 1; x <= width; x++) {
        to[y][x] = from[y][x];
        if (!from[y][x]) continue;
        int adjacent = 0;
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (from[y + dy][x + dx]) adjacent++;
          }
        }
        if (adjacent < 4) {
          to[y][x] = false;
          num_removed++;
        }
      }
    }

    if (num_removed == 0) break;
    if (iteration == 0) part1 += num_removed;
    part2 += num_removed;
    iteration++;
    if (iteration == max_size) die("too long");
  }

  print_uints(part1, part2);
}
