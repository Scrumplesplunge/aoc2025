#include "core/assert.h"
#include "core/io.h"

typedef unsigned short node_id;

enum { max_outs = 2048 };
int num_outs;
node_id outs[max_outs];

enum { has_dac = 1, has_fft = 2 };
struct device {
  unsigned short num_ins;
  unsigned short num_outs;
  node_id* outs;
  unsigned short part1;
  unsigned long long part2[4];
};

enum { max_devices = 26 * 26 * 26 };
struct device devices[max_devices];

bool is_lower(char c) { return 'a' <= c && c <= 'z'; }

node_id id(const char* key) {
  if (!is_lower(key[0]) || !is_lower(key[1]) || !is_lower(key[2])) {
    die("bad input id");
  }
  node_id result = 0;
  for (int i = 0; i < 3; i++) result = 26 * result + (key[i] - 'a');
  return result;
}

void read_input() {
  enum { buffer_size = 11 << 10 };
  char buffer[buffer_size];
  const int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input");

  const char* i = buffer;
  const char* end = i + n;

  while (i != end) {
    const node_id d = id(i);
    i += 3;
    if (*i++ != ':') die("bad input");
    const int first_out = num_outs;
    while (*i != '\n') {
      if (*i++ != ' ') die("bad input");
      const node_id o = id(i);
      if (num_outs == max_outs) die("bad input");
      outs[num_outs++] = o;
      devices[o].num_ins++;
      i += 3;
    }
    assert(*i == '\n');
    i++;
    devices[d].num_outs = num_outs - first_out;
    devices[d].outs = outs + first_out;
  }
}

int main() {
  read_input();
  int head = 0, tail = 0;
  node_id queue[max_devices];
  devices[id("you")].part1 = 1;
  devices[id("svr")].part2[0] = 1;
  for (int i = 0; i < max_devices; i++) {
    if (devices[i].num_outs > 0 && devices[i].num_ins == 0) queue[tail++] = i;
  }
  while (head != tail) {
    const int i = queue[head++];
    struct device* d = &devices[i];
    if (i == id("dac")) {
      d->part2[has_dac] += d->part2[0];
      d->part2[has_dac | has_fft] += d->part2[has_fft];
    } else if (i == id("fft")) {
      d->part2[has_fft] += d->part2[0];
      d->part2[has_fft | has_dac] += d->part2[has_dac];
    }
    for (int i = 0; i < d->num_outs; i++) {
      struct device* o = &devices[d->outs[i]];
      assert(o->num_ins > 0);
      o->part1 += d->part1;
      for (int i = 0; i < 4; i++) o->part2[i] += d->part2[i];
      o->num_ins--;
      if (o->num_ins == 0) queue[tail++] = d->outs[i];
    }
  }
  const struct device* out = &devices[id("out")];
  print_ulongs(out->part1, out->part2[has_dac | has_fft]);
}
