#include "core/assert.h"
#include "core/io.h"

struct point {
  int x, y;
};
enum { max_points = 1000 };
int num_points;
struct point points[max_points];

void read_input() {
  enum { buffer_size = 32 << 10 };
  char buffer[buffer_size];
  const int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input");

  const char* i = buffer;
  const char* const end = i + n;

  while (i != end) {
    unsigned int x, y;
    i = scan_uint(i, &x);
    if (!i || *i++ != ',') die("bad input");
    i = scan_uint(i, &y);
    if (!i || *i++ != '\n') die("bad input");
    if (num_points == max_points) die("bad input");
    points[num_points++] = (struct point){x, y};
  }
}

long long llabs(long long x) { return x < 0 ? -x : x; }

unsigned long long part1() {
  unsigned long long largest_area = 0;
  const int n = num_points;
  for (int i = 0; i < n; i++) {
    const struct point a = points[i];
    for (int j = i + 1; j < n; j++) {
      const struct point b = points[j];
      const long long dx = a.x - b.x;
      const long long dy = a.y - b.y;
      const unsigned long long area = (llabs(dx) + 1) * (llabs(dy) + 1);
      if (area > largest_area) largest_area = area;
    }
  }
  return largest_area;
}

long long clockwise_area() {
  long long area = 0;
  for (int i = 1, n = num_points; i < n; i++) {
    const struct point a = points[i - 1];
    const struct point b = points[i];
    if (a.x != b.x) area -= a.y * (b.x - a.x);
  }
  return area;
}

enum {
  large_grid_size = 100000,    // coord limit in expanded space
  condensed_grid_size = 1000,  // coord limit in condensed grid
};
unsigned short xs[large_grid_size];
unsigned short ys[large_grid_size];
char grid[condensed_grid_size][condensed_grid_size];

struct vec { unsigned short x, y; };
struct vec queue[4 * condensed_grid_size * condensed_grid_size];
unsigned long long part2() {
  // Identify all populated rows and columns.
  const int n = num_points;
  for (int i = 0; i < n; i++) {
    const struct point p = points[i];
    xs[p.x] = 1;
    ys[p.y] = 1;
  }
  // Assign condensed coordinates to populated rows and columns.
  unsigned short size_x = 0, size_y = 0;
  for (int i = 0; i < large_grid_size; i++) {
    if (xs[i]) {
      xs[i] = size_x + 1;
      size_x += 2;
    } else {
      xs[i] = size_x;
    }
    if (ys[i]) {
      ys[i] = size_y + 1;
      size_y += 2;
    } else {
      ys[i] = size_y;
    }
  }
  assert(++size_x <= condensed_grid_size);
  assert(++size_y <= condensed_grid_size);
  memset(grid, '.', sizeof(grid));
  const bool is_clockwise = clockwise_area() > 0;
  // Render the condensed grid with a border of `#` and a layer of `!` on the
  // inside edge. The `!` will be used to seed a flood-fill later.
  for (int i = 0; i < n; i++) {
    const struct point a = points[i == 0 ? n - 1 : i - 1];
    const struct point b = points[i];
    if (a.x == b.x) {
      const int x = xs[a.x];
      const int ay = ys[a.y], by = ys[b.y];
      const int step = ay <= by ? 1 : -1;
      const int end = by + step;
      const int inside_offset = is_clockwise ? -step : step;
      for (int y = ay; y != end; y += step) {
        grid[y][x] = '#';
        if (grid[y][x + inside_offset] == '.') grid[y][x + inside_offset] = '!';
      }
    } else {
      const int y = ys[a.y];
      const int ax = xs[a.x], bx = xs[b.x];
      const int step = ax <= bx ? 1 : -1;
      const int end = bx + step;
      const int inside_offset = is_clockwise ? step : -step;
      for (int x = ax; x != end; x += step) {
        grid[y][x] = '#';
        if (grid[y + inside_offset][x] == '.') grid[y + inside_offset][x] = '!';
      }
    }
  }
  // Seed the flood-fill queue with all of the inside edge.
  int tail = 0;
  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
      if (grid[y][x] == '!') {
        grid[y][x] = '#';
        queue[tail++] = (struct vec){x, y};
      }
    }
  }
  // Fill the inside area.
  int head = 0;
  while (head < tail) {
    const struct vec p = queue[head++];
    assert(p.x > 0 && p.y > 0);
    assert(tail < condensed_grid_size * condensed_grid_size);
    const struct vec neighbours[] = {
        {p.x, p.y - 1}, {p.x, p.y + 1}, {p.x - 1, p.y}, {p.x + 1, p.y}};
    for (int i = 0; i < 4; i++) {
      const struct vec n = neighbours[i];
      if (grid[n.y][n.x] == '.') {
        grid[n.y][n.x] = '#';
        queue[tail++] = n;
      }
    }
  }
  // Check every candidate.
  unsigned long long largest_area = 0;
  for (int i = 0; i < n; i++) {
    const struct point a = points[i];
    const int ax = xs[a.x], ay = ys[a.y];
    for (int j = i + 1; j < n; j++) {
      const struct point b = points[j];
      const int bx = xs[b.x], by = ys[b.y];
      const int min_x = ax < bx ? ax : bx;
      const int max_x = ax < bx ? bx : ax;
      const int min_y = ay < by ? ay : by;
      const int max_y = ay < by ? by : ay;
      // Check that a candidate is valid by checking that its entire border is
      // filled in. This would fail for awkward inputs where there is a hole in
      // the middle, but it works for the actual input types.
      bool valid = true;
      for (int y = min_y; y <= max_y; y++) {
        if (grid[y][min_x] != '#' || grid[y][max_x] != '#') {
          valid = false;
          goto stahp;
        }
      }
      for (int x = min_x; x <= max_x; x++) {
        if (grid[min_y][x] != '#' || grid[max_y][x] != '#') {
          valid = false;
          goto stahp;
        }
      }
stahp:
      if (valid) {
        const long long dx = a.x - b.x;
        const long long dy = a.y - b.y;
        const unsigned long long area = (llabs(dx) + 1) * (llabs(dy) + 1);
        if (area > largest_area) largest_area = area;
      }
    }
  }
  return largest_area;
}

int main() {
  read_input();
  print_ulongs(part1(), part2());
}
