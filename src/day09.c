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

// Original plan (doesn't work):
//
// Handle border thickness by:
//
//   * Initially treating borders as excluded.
//   * Buffing the biggest rect by +1 on all sides.
//
// Overall plan:
//
//   * Build a list of rectangles describing the inside area. This is done by
//     iteratively adding or removing rectangles which span all the way from
//     x=0 out to two given vertices on a vertical edge and keeping track of
//     the number of layers in any given subrectangle. Once all edges are
//     processed, there should only be rectangles with density 1 or -1 left,
//     depending on whether the vertices are ordered clockwise or anticlockwise.
//   * Iterate over all pairs of vertices and ignore any rectangle which is not
//     entirely inside the area covered by the rectangle list.

// struct rect {
//   int x_min, x_max, y_min, y_max, count;
// };
// enum { max_rects = 2000 };
// int num_rects;
// struct rect rects[max_rects];
// 
// bool check_rects() {
//   for (int i = 0, n = num_rects; i < n; i++) {
//     const struct rect a = rects[i];
//     for (int j = i + 1; j < n; j++) {
//       const struct rect b = rects[j];
//       if (a.x_max <= b.x_min || b.x_max <= a.x_min ||
//           a.y_max <= b.y_min || b.y_max <= a.y_min) {
//         // Rectangles do not overlap.
//         continue;
//       }
//       die("bug: overlap in rect list");
//     }
//   }
//   return true;
// }
// 
// void add_rect(struct rect new_rect) {
//   const int initial_num_rects = num_rects;
//   // Partially or fully remove rects which overlap the argument.
//   int j = 0;
//   for (int i = 0; i < initial_num_rects; i++) {
//     struct rect rect = rects[i];
//     if (rect.x_max <= new_rect.x_min || new_rect.x_max <= rect.x_min ||
//         rect.y_max <= new_rect.y_min || new_rect.y_max <= rect.y_min) {
//       // Rectangles do not overlap.
//       rects[j++] = rect;
//       continue;
//     }
//     if (new_rect.x_min <= rect.x_min && rect.x_max <= new_rect.x_max &&
//         new_rect.y_min <= rect.y_min && rect.y_max <= new_rect.y_max) {
//       // Fully new_rect.
//       continue;
//     }
//     if (num_rects + 4 > max_rects) die("bad input too big");
//     if (rect.y_min < new_rect.y_min) {
//       struct rect above = rect;
//       above.y_max = new_rect.y_min;
//       rects[num_rects++] = above;
//       rect.y_min = new_rect.y_min;
//     }
//     if (new_rect.y_max < rect.y_max) {
//       struct rect below = rect;
//       below.y_min = new_rect.y_max;
//       rects[num_rects++] = below;
//       rect.y_max = new_rect.y_max;
//     }
//     if (rect.x_min < new_rect.x_min) {
//       struct rect left = rect;
//       left.x_max = new_rect.x_min;
//       rects[num_rects++] = left;
//       rect.x_min = new_rect.x_min;
//     }
//     if (new_rect.x_max < rect.x_max) {
//       struct rect right = rect;
//       right.x_min = new_rect.x_max;
//       rects[num_rects++] = right;
//       rect.x_max = new_rect.x_min;
//     }
//     rect.count += new_rect.count;
//     // nah mate this bit would be too hard -- need to also handle fragmenting
//     // the remaining parts of the new_rect...
//     if (
//   }
//   while (j < initial_num_rects && initial_num_rects < num_rects) {
//     rects[j++] = rects[--num_rects];
//   }
//   assert(check_rects());
// }
// 
// void add_rect(struct rect x) {
//   if (num_rects == max_rects) die("bad input");
//   remove_rect(x);
//   rects[num_rects++] = x;
//   assert(check_rects());
// }
// 
// unsigned long long part2() {
//   // Ignoring border thickness, compute the contained area.
//   const int n = num_points;
//   for (int i = 1; i < n; i++) {
//     const struct point a = points[i - 1];
//     const struct point b = points[i];
//     assert(a.x == b.x || a.y == b.y);
//     if (a.x != b.x) continue;
//     if (a.y < b.y) {
//       add_rect((struct rect){
//           .x_min = 0,
//           .x_max = a.x,
//           .y_min = a.y,
//           .y_max = b.y,
//       });
//     } else {
//       remove_rect((struct rect){
//           .x_min = 0,
//           .x_max = a.x,
//           .y_min = b.y,
//           .y_max = a.y,
//       });
//     }
//   }
// }

// New plan:
//
//   * Compact the grid by condensing blank rows or columns into a single
//     representative. This works because any consecutive sequence of blank
//     rows or columns must be identical.
//   * Flood-fill the outside area doesn't work because there might be holes.

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
  print("condensed size: (%d, %d)\n", size_x, size_y);
  const bool is_clockwise = clockwise_area() > 0;
  print("grid is %s\n", is_clockwise ? "clockwise" : "anticlockwise");
  // Render the condensed grid.
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
  // Fill the inside area.
  int tail = 0;
  for (int y = 0; y < size_y; y++) {
    for (int x = 0; x < size_x; x++) {
      if (grid[y][x] == '!') {
        grid[y][x] = '#';
        queue[tail++] = (struct vec){x, y};
      }
    }
  }
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
      bool valid = true;
      for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
          if (grid[y][x] != '#') {
            valid = false;
            goto stahp;
          }
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

// New new plan:
//
//   * figure out whether the input is clockwise or not by computing the area.
//   * next, build rectangles representing "inside".
//   * finally, check all candidates against "inside".
//   * but hard

int main() {
  read_input();
  print_ulongs(part1(), part2());
}
