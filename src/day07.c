#include "core/assert.h"
#include "core/io.h"

enum { buffer_size = 21 << 10 };
char buffer[buffer_size];

enum { max_beams = 141 };
struct beam {
  // Column containing the beam.
  unsigned long long x : 8;
  // Number of different timelines leading to a beam in this column.
  unsigned long long num_timelines : 56;
};
struct beam_row {
  int num_beams;
  struct beam beams[max_beams];
};
struct beam_row beams[2];

int add_beam(struct beam* beams, int num_beams, int x,
             unsigned long long num_timelines) {
  assert(num_beams == 0 || beams[num_beams - 1].x <= x);
  if (num_beams > 0 && beams[num_beams - 1].x == x) {
    beams[num_beams - 1].num_timelines += num_timelines;
  } else {
    beams[num_beams++] = (struct beam){x, num_timelines};
  }
  return num_beams;
}

int main() {
  const int length = read(STDIN_FILENO, buffer, buffer_size);
  if (length == 0 || buffer[length - 1] != '\n') die("bad input");

  // Determine the grid dimensions.
  int width = 0;
  while (buffer[width] != '\n') width++;
  const int height = length / (width + 1);
  if (length % (width + 1) != 0) die("bad input");

  for (int i = 0; i < width; i++) {
    if (buffer[i] == 'S') {
      beams[0].beams[beams[0].num_beams++] =
          (struct beam){.x = i, .num_timelines = 1};
      break;
    }
  }

  unsigned int part1 = 0;
  for (int y = 1; y < height; y++) {
    const struct beam_row* incoming = &beams[(y - 1) % 2];
    struct beam_row* outgoing = &beams[y % 2];
    const int num_incoming_beams = incoming->num_beams;
    const char* row = buffer + y * (width + 1);
    int n = 0;
    for (int i = 0; i < num_incoming_beams; i++) {
      const int x = incoming->beams[i].x;
      const unsigned long long num_timelines = incoming->beams[i].num_timelines;
      if (row[x] == '^') {
        // Beam is split.
        part1++;
        assert(1 <= x && x < width - 1);
        n = add_beam(outgoing->beams, n, x - 1, num_timelines);
        n = add_beam(outgoing->beams, n, x + 1, num_timelines);
      } else {
        n = add_beam(outgoing->beams, n, x, num_timelines);
      }
    }
    outgoing->num_beams = n;
  }

  // Calculate the total number of timelines by summing up the number of
  // timelines for each possible beam column in the bottom row.
  unsigned long long part2 = 0;
  const struct beam_row* final_row = &beams[(height - 1) % 2];
  for (int i = 0; i < final_row->num_beams; i++) {
    part2 += final_row->beams[i].num_timelines;
  }

  print_ulongs(part1, part2);
}
