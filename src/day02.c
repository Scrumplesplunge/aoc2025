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

static const unsigned int pow10_data[] = {1, 10, 100, 1000, 10000, 100000};
static unsigned int pow10(int n) {
  assert(0 <= n && n < sizeof(pow10_data) / sizeof(pow10_data[0]));
  return pow10_data[n];
}

int main() {
  char buffer[1 << 10];
  const int length = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (length == 0 || length == sizeof(buffer) || buffer[length - 1] != '\n') {
    die("bad input l");
  }
  const char* i = buffer;
  const char* const end = buffer + length;
  unsigned long long invalid_id_sum = 0;
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
    if (first_n == second_n) {
      // Every number needs to consist of a value repeated twice, so we're only
      // interested in numbers with even length.
      if (first_n % 2 == 1) continue;
      const int chunk_n = first_n / 2;
      const unsigned int first_a = read_uint_n(first, chunk_n);
      const unsigned int first_b = read_uint_n(first + chunk_n, chunk_n);
      const unsigned int second_a = read_uint_n(second, chunk_n);
      const unsigned int second_b = read_uint_n(second + chunk_n, chunk_n);
      // The first possible repeating value needs to be at least:
      const unsigned int a = first_b <= first_a ? first_a : first_a + 1;
      // The last possible repeating value needs to be at most:
      const unsigned int b = second_a <= second_b ? second_a : second_a - 1;
      if (a > b) continue;
      // Calculate the sum of all invalid IDs in this range.
      const unsigned long long x = b * (b + 1) / 2 - (a - 1) * a / 2;
      invalid_id_sum += x + pow10(chunk_n) * x;
      print("%u%u-%u%u: %u-%u\n", first_a, first_b, second_a, second_b, a, b);
    } else if (first_n % 2 == 0) {
      assert(first_n + 1 == second_n);
      const int chunk_n = first_n / 2;
      const unsigned int first_a = read_uint_n(first, chunk_n);
      const unsigned int first_b = read_uint_n(first + chunk_n, chunk_n);
      // The first possible repeating value needs to be at least:
      const unsigned int a = first_b <= first_a ? first_a : first_a + 1;
      // The last possible repeating value needs to be at most:
      const unsigned int b = pow10(chunk_n) - 1;
      if (a > b) continue;
      // Calculate the sum of all invalid IDs in this range.
      const unsigned long long x = b * (b + 1) / 2 - (a - 1) * a / 2;
      invalid_id_sum += x + pow10(chunk_n) * x;
      print("%u%u-%u%u: %u-%u\n", first_a, first_b, b, b, a, b);
    } else if (second_n % 2 == 0) {
      assert(first_n + 1 == second_n);
      const int chunk_n = second_n / 2;
      const unsigned int second_a = read_uint_n(second, chunk_n);
      const unsigned int second_b = read_uint_n(second + chunk_n, chunk_n);
      // The first possible repeating value needs to be at least:
      const unsigned int a = pow10(chunk_n - 1);
      // The last possible repeating value needs to be at most:
      const unsigned int b = second_a <= second_b ? second_a : second_a - 1;
      if (a > b) continue;
      // Calculate the sum of all invalid IDs in this range.
      const unsigned long long x = b * (b + 1) / 2 - (a - 1) * a / 2;
      invalid_id_sum += x + pow10(chunk_n) * x;
      print("%u%u-%u%u: %u-%u\n", a, a, second_a, second_b, a, b);
    } else {
      die("oh no");
    }
  }
  print_uints(invalid_id_sum, 0);
}

// 423897767 wrong answer (too low)
