#include "core/assert.h"
#include "core/io.h"

enum { buffer_size = 21 << 10 };
char buffer[buffer_size];

enum { max_beams = 141 };
int num_beams[2];
unsigned char beams[2][max_beams];

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
      beams[0][num_beams[0]++] = i;
      break;
    }
  }

  unsigned int part1 = 0;
  for (int y = 1; y < height; y++) {
    const int num_incoming_beams = num_beams[(y - 1) % 2];
    const unsigned char* incoming_beams = beams[(y - 1) % 2];
    unsigned char* outgoing_beams = beams[y % 2];
    const char* row = buffer + y * (width + 1);
    int num_outgoing_beams = 0;
    for (int i = 0; i < num_incoming_beams; i++) {
      const int x = incoming_beams[i];
      if (x < 1 || width - 1 <= x) {
        print("row y=%d beam x=%d\n", y, x);
      }
      if (row[x] == '^') {
        part1++;
        assert(1 <= x && x < width - 1);
        // Avoid re-adding a beam that already exists.
        if (num_outgoing_beams == 0 ||
            outgoing_beams[num_outgoing_beams - 1] < x - 1) {
          outgoing_beams[num_outgoing_beams++] = x - 1;
        }
        outgoing_beams[num_outgoing_beams++] = x + 1;
      } else {
        if (num_outgoing_beams == 0 ||
            outgoing_beams[num_outgoing_beams - 1] < x) {
          outgoing_beams[num_outgoing_beams++] = x;
        }
      }
    }
    num_beams[y % 2] = num_outgoing_beams;
    print("%d beams:", num_outgoing_beams);
    for (int i = 0; i < num_outgoing_beams; i++) {
      print("\t%d", outgoing_beams[i]);
    }
    print("\n");
  }

  print_uints(part1, 0);
}
