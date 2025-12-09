#include "core/io.h"

struct point { int x, y; };
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

int main() {
  read_input();

  unsigned long long part1 = 0;
  const int n = num_points;
  for (int i = 0; i < n; i++) {
    const struct point a = points[i];
    for (int j = i + 1; j < n; j++) {
      const struct point b = points[j];
      const long long dx = a.x - b.x;
      const long long dy = a.y - b.y;
      const unsigned long long area = (llabs(dx) + 1) * (llabs(dy) + 1);
      if (area > part1) part1 = area;
    }
  }

  print_ulongs(part1, 0);
}
