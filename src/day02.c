#include "core/assert.h"
#include "core/io.h"

static unsigned int read_uint_n(const char* in, int n) {
  unsigned int result = 0;
  for (int i = 0; i < n; i++) {
    assert(is_digit(in[i]));
    result = 10 * result + (in[i] - '0');
  }
  return result;
}

static const unsigned long long pow10_data[] = {
    1,       10,       100,       1000,       10000,      100000,
    1000000, 10000000, 100000000, 1000000000, 10000000000};
static unsigned long long pow10(int n) {
  assert(0 <= n && n < sizeof(pow10_data) / sizeof(pow10_data[0]));
  return pow10_data[n];
}

// Compares a repeating sequence of a[0..an] against b[0..bn].
int compare_repeated(const char* a, int an, const char* b, int bn) {
  assert(bn % an == 0);
  for (int i = 0; i < bn; i++) {
    const int diff = a[i % an] - b[i];
    if (diff != 0) return diff;
  }
  return 0;
}

unsigned int min_chunk(const char* in, int n, int chunk_size) {
  assert(n > 0 && n % chunk_size == 0);
  unsigned int min = -1;
  for (int i = 0; i < n; i += chunk_size) {
    const unsigned int x = read_uint_n(in + i, chunk_size);
    if (x < min) min = x;
  }
  return min;
}

unsigned int max_chunk(const char* in, int n, int chunk_size) {
  assert(n > 0 && n % chunk_size == 0);
  unsigned int max = 0;
  for (int i = 0; i < n; i += chunk_size) {
    const unsigned int x = read_uint_n(in + i, chunk_size);
    if (x > max) max = x;
  }
  return max;
}

unsigned int masks[] = {0x00001, 0x00011, 0x00101, 0x01011, 0x10001};

int main() {
  char buffer[1 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input l");
  }
  const char* i = buffer;
  const char* const end = buffer + length;
  unsigned long long part1 = 0;
  unsigned long long part2 = 0;
  while (i != end) {
    // Parse the value range.
    const char* const first = i;
    while (is_digit(*i)) i++;
    if (i == first || *i != '-') {
      write(STDOUT_FILENO, i, strlen(i));
      die("bad input d");
    }
    const int first_n = i - first;
    i++;
    const char* const second = i;
    while (is_digit(*i)) i++;
    if (i == second || (*i != ',' && *i != '\n')) die("bad input x");
    const int second_n = i - second;
    i++;
    if (second_n < first_n || first_n + 1 < second_n) die("bad range");
    const int max_sequence_length = second_n / 2;
    assert(max_sequence_length <= 5);
    write(STDERR_FILENO, first, i - first);
    write(STDERR_FILENO, "\n", 1);

    // Consider every repeating pattern length.
    unsigned long long before = part2;
    // Consider every ID length.
    for (int n = first_n; n <= second_n; n++) {
      unsigned int mask = 0x00000;
      for (int l = max_sequence_length; l >= 1; l--) {
        if (n % l != 0 || n / l == 1) {
          mask &= ~(0xFu << (4 * (l - 1)));
          continue;
        }
        unsigned long long a, b;
        if (n == first_n) {
          const unsigned int prefix = read_uint_n(first, l);
          a = compare_repeated(first, l, first, first_n) >= 0 ? prefix
                                                              : prefix + 1;
        } else {
          a = pow10(l - 1);
        }
        if (n == second_n) {
          const unsigned int prefix = read_uint_n(second, l);
          b = compare_repeated(second, l, second, second_n) <= 0 ? prefix
                                                                 : prefix - 1;
        } else {
          b = pow10(l) - 1;
        }
        if (b < a) {
          mask &= ~(0xFu << (4 * (l - 1)));
          continue;
        }
        // Number of different repeating sequences which work.
        const unsigned long long x = b * (b + 1) / 2 - (a - 1) * a / 2;
        // Sum of all IDs with one of these sequences.
        unsigned long long sum = 0;
        for (int i = 0; i < n; i += l) {
          sum += x * pow10(i);
        }
        if (n / l == 2) part1 += sum;

        // this is some bullshit.
        
        // check how many times we already counted sequences of this length.
        const int current_count = (mask >> (4 * (l - 1))) & 0xF;
        // correct it to a single count.
        part2 += (1 - current_count) * sum;
        // update the mask.
        mask += (1 - current_count) * masks[l - 1];
        const int new_count = (mask >> (4 * (l - 1))) & 0xF;
        assert(new_count == 1);
      }
      // Check that everything is counted at most once.
      for (int i = 0; i < 5; i++) {
        unsigned x = (mask >> (4 * i)) & 0xF;
        assert(x == 0 || x == 1);
      }
    }
    const unsigned long long after = part2;
    print("  +%llu\n", after - before);
  }
  print_ulongs(part1, part2);
}

// part 2 21932329060 wrong answer (too low)
