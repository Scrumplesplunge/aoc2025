#include "core/assert.h"
#include "core/io.h"

struct node {
  unsigned short parent, size;
  int x;
};

struct edge {
  unsigned long long distance;
  unsigned short a, b;
};

enum { max_nodes = 1024, max_edges = max_nodes * (max_nodes - 1) / 2 };
int num_nodes;
struct node nodes[max_nodes];
struct edge edges[max_edges];

void sift_down(struct edge* values, int n, int i) {
  const struct edge x = values[i];
  while (true) {
    int c = 2 * i + 1;
    if (c >= n) break;
    if (c + 1 < n && values[c + 1].distance > values[c].distance) c++;
    if (values[c].distance <= x.distance) break;
    values[i] = values[c];
    i = c;
  }
  values[i] = x;
}

void sort(struct edge* values, int n) {
  for (int i = n / 2; i >= 0; i--) sift_down(values, n, i);
  for (int i = n - 1; i >= 1; i--) {
    const struct edge x = values[0];
    values[0] = values[i];
    values[i] = x;
    sift_down(values, i, 0);
  }
  for (int i = 1; i < n; i++) {
    if (values[i - 1].distance > values[i].distance) die("sort is broken");
  }
}

unsigned short find_root(unsigned short node) {
  unsigned short root = node;
  while (nodes[root].parent != root) root = nodes[root].parent;
  while (node != root) {
    unsigned short temp = nodes[node].parent;
    nodes[node].parent = root;
    node = temp;
  }
  return root;
}

unsigned short merge_nodes(unsigned short a, unsigned short b) {
  a = find_root(a);
  b = find_root(b);
  if (a == b) return a;
  if (nodes[a].size >= nodes[b].size) {
    nodes[b].parent = a;
    nodes[a].size += nodes[b].size;
    return a;
  } else {
    nodes[a].parent = b;
    nodes[b].size += nodes[a].size;
    return b;
  }
}

enum { max_points = 1000 };
struct point { int x, y, z; };

int read_points(struct point* points) {
  enum { buffer_size = 32 << 10 };
  char buffer[buffer_size];
  const int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input");

  const char* i = buffer;
  const char* const end = i + n;

  int num_points = 0;
  while (i != end) {
    unsigned int x, y, z;
    i = scan_uint(i, &x);
    if (!i || *i++ != ',') die("bad input");
    i = scan_uint(i, &y);
    if (!i || *i++ != ',') die("bad input");
    i = scan_uint(i, &z);
    if (!i || *i++ != '\n') die("bad input");
    if (num_points == max_points) die("bad input");
    points[num_points++] = (struct point){x, y, z};
  }
  return num_points;
}

void read_input() {
  struct point points[max_points];
  const int num_points = read_points(points);
  num_nodes = num_points;
  int num_edges = 0;
  for (int a = 0; a < num_points; a++) {
    nodes[a] = (struct node){a, 1, points[a].x};
    for (int b = a + 1; b < num_points; b++) {
      const long long dx = points[a].x - points[b].x;
      const long long dy = points[a].y - points[b].y;
      const long long dz = points[a].z - points[b].z;
      const unsigned long long distance = dx * dx + dy * dy + dz * dz;
      edges[num_edges++] = (struct edge){distance, a, b};
    }
  }
  assert(num_edges == num_nodes * (num_nodes - 1) / 2);
  sort(edges, num_edges);
}

unsigned short circuits[max_nodes];
unsigned long long part1() {
  if (num_nodes * (num_nodes - 1) / 2 < 1000) die("bad input count");
  for (int i = 0; i < 1000; i++) {
    const struct edge e = edges[i];
    merge_nodes(e.a, e.b);
  }
  // Enumerate all the roots.
  int num_circuits = 0;
  for (int i = 0; i < num_nodes; i++) {
    if (find_root(i) == i) circuits[num_circuits++] = i;
  }
  if (num_circuits < 3) die("bad input");
  // Find the biggest three.
  unsigned short big[3];
  for (int i = 0; i < 3; i++) {
    unsigned short biggest = 0;
    for (int j = 1; j < num_circuits; j++) {
      if (nodes[circuits[j]].size > nodes[circuits[biggest]].size) biggest = j;
    }
    big[i] = circuits[biggest];
    circuits[biggest] = circuits[--num_circuits];
  }
  return nodes[big[0]].size * nodes[big[1]].size * nodes[big[2]].size;
}

unsigned long long part2() {
  const int num_edges = num_nodes * (num_nodes - 1) / 2;
  for (int i = 1000; i < num_edges; i++) {
    const struct edge e = edges[i];
    const unsigned short root = merge_nodes(e.a, e.b);
    if (nodes[root].size == num_nodes) {
      const unsigned long long a = nodes[e.a].x;
      const unsigned long long b = nodes[e.b].x;
      return a * b;
    }
  }
  die("bug");
}

int main() {
  read_input();
  const unsigned long long a = part1();
  const unsigned long long b = part2();
  print_ulongs(a, b);
}
