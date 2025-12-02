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

static unsigned long long pow10ull(unsigned n) {
  assert(n < sizeof(pow10_data) / sizeof(pow10_data[0]));
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

// For part 2, we need a way to avoid double-counting values in each range.
// To do this, we can attribute each ID to a logical bucket corresponding to the
// shortest repeating sequence length which applies for that ID. When we count
// up all the invalid IDs with a repeating group of length N, we may also be
// counting some IDs for logical buckets <N. For example, the count for length 4
// will also count values for lengths 2 and 1.
//
// To work around this, we can keep track of how many times we have counted a
// specific ID and ensure that we have no double-counting by the end. For each
// possible repeating pattern length (1-5), we have a hard-coded list of other
// pattern lengths which will be counted (e.g. for 4, we have 4, 2, and 1). We
// represent this with a single unsigned int which represents a vector of 4-bit
// counters (so the value for 4 has 4+2+1 -> 0x01011).
//
// This allows us to easily keep track of what we've counted by performing
// arithmetic on the masks.
static const unsigned int masks[] = {0x00001, 0x00011, 0x00101, 0x01011,
                                     0x10001};

int main() {
  char buffer[1 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input");
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
      die("bad input");
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

    // Consider every ID length.
    for (int n = first_n; n <= second_n; n++) {
      // Mask of pattern lengths that we have counted (initially nothing).
      unsigned int mask = 0x00000;
      // Consider every repeating pattern length.
      for (int l = max_sequence_length; l >= 1; l--) {
        if (n % l != 0 || n / l == 1) {
          // If `l` does not divide the length then there are no invalid IDs
          // in this logical bucket, so there's nothing to count
          // (and double-counting 0 is still 0, so nothing to fix).
          mask &= ~(0xFu << (4 * (l - 1)));
          continue;
        }
        // Rather than iterating over all possible repeating sequences of length
        // `l`, we can calculate the minimum value `a` that would work and the
        // maximum value `b` that would work and then derive the sum from that.
        unsigned long long a, b;
        if (n == first_n) {
          const unsigned int prefix = read_uint_n(first, l);
          a = compare_repeated(first, l, first, first_n) >= 0 ? prefix
                                                              : prefix + 1;
        } else {
          a = pow10ull(l - 1);
        }
        if (n == second_n) {
          const unsigned int prefix = read_uint_n(second, l);
          b = compare_repeated(second, l, second, second_n) <= 0 ? prefix
                                                                 : prefix - 1;
        } else {
          b = pow10ull(l) - 1;
        }
        if (b < a) {
          // There are no eligible patterns with this length, so there's nothing
          // to count (and double-counting 0 is still 0, so nothing to fix).
          mask &= ~(0xFu << (4 * (l - 1)));
          continue;
        }
        // We want to calculate the sum of all invalid IDs, which is the sum of
        // all sequences of `k` repeats of the values in the range `[a, b]`.
        // We can get this by calculating the sum of the range `[a, b]` and then
        // extrapolating to the sum of the repeated sequences.
        const unsigned long long x = b * (b + 1) / 2 - (a - 1) * a / 2;
        // Sum of all IDs with one of these sequences.
        unsigned long long sum = 0;
        for (int i = 0; i < n; i += l) sum += x * pow10ull(i);
        if (n / l == 2) part1 += sum;

        // Check how many times we already counted sequences of this length,
        // using the double-counting mask.
        const int current_count = (mask >> (4 * (l - 1))) & 0xF;
        // Add (or subtract) multiples so that this count is correct. In doing
        // so, we may also influence the number of times that we counted shorter
        // sequences, but since we're iterating from longest to shortest we will
        // still have chance to correct that before finishing.
        part2 += (1 - current_count) * sum;
        mask += (1 - current_count) * masks[l - 1];
        assert(((mask >> (4 * (l - 1))) & 0xF) == 1);
      }
#ifndef NDEBUG
      // Check that everything is counted at most once.
      for (int i = 0; i < 5; i++) {
        unsigned x = (mask >> (4 * i)) & 0xF;
        assert(x == 0 || x == 1);
      }
#endif
    }
  }
  print_ulongs(part1, part2);
}
