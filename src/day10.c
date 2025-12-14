#include "core/assert.h"
#include "core/io.h"

typedef unsigned short uint16;

enum { max_buttons = 14, max_size = 16 };
struct machine {
  uint16 size;
  uint16 target;
  uint16 num_buttons;
  uint16 buttons[max_buttons];
  uint16 joltages[max_size];
};

enum { max_machines = 200 };
long long int num_machines;
struct machine machines[max_machines];

void read_input() {
  enum { buffer_size = 21 << 10 };
  char buffer[buffer_size];
  const long long int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input len");

  const char* i = buffer;
  const char* end = i + n;

  while (i != end) {
    if (num_machines == max_machines) die("bad input machine count");
    struct machine* const m = &machines[num_machines++];
    if (*i++ != '[') die("bad input");
    // Read the target light sequence.
    unsigned long long int next_bit = 1;
    while (*i == '.' || *i == '#') {
      if (next_bit == (1 << max_size)) die("bad input mask size");
      if (*i == '#') m->target |= next_bit;
      next_bit <<= 1;
      i++;
    }
    if (*i++ != ']' || *i++ != ' ' || *i++ != '(') die("bad input syntax");
    // Read the set of buttons.
    while (true) {
      if (m->num_buttons == max_buttons) die("bad input button count");
      uint16* const button = &m->buttons[m->num_buttons++];
      while (true) {
        if (!is_digit(*i)) die("bad input syntax2");
        *button |= 1 << (*i++ - '0');
        if (*i != ',') break;
        i++;
      }
      if (*i++ != ')' || *i++ != ' ') die("bad input syntax3");
      if (*i != '(') break;
      i++;
    }
    if (*i++ != '{') die("bad input syntax4");
    // Skip the joltages for now.
    do {
      if (m->size == max_size) die("bad input joltage count");
      unsigned int value;
      i = scan_uint(i, &value);
      if (!i) die("bad input syntax6");
      m->joltages[m->size++] = value;
      if (*i != '}' && *i != ',') die("bad input syntax7");
    } while (*i++ == ',');
    if (*i++ != '\n') die("bad input syntax5");
  }
}

long long int popcount(unsigned long long int x) {
  long long int total;
  asm("popcnt %1, %0" : "=r"(total) : "r"(x));
  return total;
}

long long int part1() {
  long long int total = 0;
  for (long long int i = 0, n = num_machines; i < n; i++) {
    const struct machine* m = &machines[i];
    long long int best_count = max_buttons + 1;
    for (long long int set = 0, all = (1 << m->num_buttons) - 1; set < all; set++) {
      uint16 result = 0;
      for (long long int i = 0; i < max_buttons; i++) {
        if (set & (1 << i)) result ^= m->buttons[i];
      }
      if (result != m->target) continue;
      const long long int count = popcount(set);
      if (count >= best_count) continue;
      best_count = count;
    }
    total += best_count;
  }
  return total;
}

enum { max_rows = 1024, max_columns = 1024 };
struct table {
  // Number of rows in the Tableau.
  long long int num_rows;
  // Number of columns **including the RHS**
  long long int num_columns;
  // Simplex Tableau:
  // | b1 .. bN I 0 | t |
  // | -1 .. -1 0 1 | k |
  long long int cells[max_rows][max_columns];
};

void swap(long long int* a, long long int* b) {
  long long int temp = *a;
  *a = *b;
  *b = temp;
}

void swap_rows(struct table* t, long long int a, long long int b) {
  for (long long int i = 0, c = t->num_columns; i < c; i++) {
    swap(&t->cells[a][i], &t->cells[b][i]);
  }
}

long long int abs(long long int x) { return x < 0 ? -x : x; }

long long int gcd(long long int a, long long int b) {
  while (b != 0) {
    const long long int temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

void reduce_row(struct table* t, long long int row) {
  long long int divisor = 0;
  const long long int n = t->num_columns;
  for (long long int i = 0; i < n; i++) {
    const long long int v = abs(t->cells[row][i]);
    if (v == 0) continue;
    if (divisor == 0) {
      divisor = v;
    } else {
      divisor = gcd(divisor, v);
    }
  }
  if (divisor == 0) return;
  for (long long int i = 0; i < n; i++) t->cells[row][i] /= divisor;
}

long long int pivot_column(const struct table* table) {
  const long long int* cost_row = table->cells[table->num_rows - 1];
  const long long int n = table->num_columns - 2;
  long long int best = -1;
  long long int best_value = 0;
  for (long long int i = 0; i < n; i++) {
    if (cost_row[i] > best_value) {
      best = i;
      best_value = cost_row[i];
    }
  }
  return best;
}

long long int pivot_row(const struct table* table, long long int column) {
  long long int best = -1;
  long long int best_n = 0, best_d = 1;
  for (long long int i = 0, r = table->num_rows - 1; i < r; i++) {
    const long long int d = table->cells[i][column];
    if (d <= 0) continue;
    const long long int n = table->cells[i][table->num_columns - 1];
    if (best == -1 || best_d * n < best_n * d) {
      best = i;
      best_n = n;
      best_d = d;
    }
  }
  return best;
}

void pivot(struct table* table, int row, int column) {
  long long int* pivot_row = table->cells[row];
  if (pivot_row[column] < 0) {
    for (long long int i = 0; i < table->num_columns; i++) {
      pivot_row[i] = -pivot_row[i];
    }
  }
  const long long int pivot_factor = pivot_row[column];
  for (long long int i = 0; i < table->num_rows; i++) {
    if (i == row) continue;
    long long int* row = table->cells[i];
    const long long int row_factor = row[column];
    if (row_factor == 0) continue;
    for (long long int j = 0; j < table->num_columns; j++) {
      const long long int result =
          pivot_factor * row[j] - row_factor * pivot_row[j];
      row[j] = result;
    }
    assert(row[column] == 0);
    reduce_row(table, i);
  }
}

void simplex_minimize(struct table* table) {
  while (true) {
    const long long int c = pivot_column(table);
    if (c == -1) break;
    const long long int r = pivot_row(table, c);
    if (r == -1) die("unsolvable");
    pivot(table, r, c);
  }
}

long long int dual_pivot_row(const struct table* table) {
  const long long int c = table->num_columns - 1;
  const long long int n = table->num_rows - 1;
  long long int best = -1;
  long long int best_value = 0;
  for (long long int i = 0; i < n; i++) {
    if (table->cells[i][c] < best_value) {
      best = i;
      best_value = table->cells[i][c];
    }
  }
  return best;
}

long long int dual_pivot_column(const struct table* table,
                                long long int row) {
  long long int best = -1;
  long long int best_n = 0, best_d = -1;
  for (long long int i = 0, c = table->num_columns - 2; i < c; i++) {
    const long long int d = table->cells[row][i];
    if (d >= 0) continue;
    const long long int n = table->cells[table->num_rows - 1][i];
    assert(n <= 0);
    if (best == -1 || best_d * n < best_n * d) {
      best = i;
      best_n = n;
      best_d = d;
    }
  }
  return best;
}

void dual_simplex(struct table* table) {
  while (true) {
    const long long int r = dual_pivot_row(table);
    if (r == -1) break;
    const long long int c = dual_pivot_column(table, r);
    if (c == -1) die("unsolvable");
    pivot(table, r, c);
  }
}

void build_table(struct table* table, const struct machine* machine) {
  for (long long int i = 0; i < max_rows; i++) {
    for (long long int j = 0; j < max_columns; j++) {
      table->cells[i][j] = 999999;
    }
  }
  const long long int s = machine->size;
  const long long int b = machine->num_buttons;
  // Build the tableau.
  table->num_rows = s + 1;
  table->num_columns = b + 2;
  for (long long int i = 0; i < b; i++) {
    for (long long int j = 0; j < s; j++) {
      table->cells[j][i] = (machine->buttons[i] >> j) & 1;
    }
    table->cells[s][i] = -1;
  }
  for (long long int i = 0; i < s; i++) {
    table->cells[i][b] = 0;
    table->cells[i][b + 1] = machine->joltages[i];
  }
  table->cells[table->num_rows - 1][table->num_columns - 2] = 1;
  table->cells[table->num_rows - 1][table->num_columns - 1] = 0;
}

// If the column is non-basic, returns -1. Otherwise, returns the row containing
// the non-zero value.
int basic_row(const struct table* table, int column) {
  int i = 0;
  const int n = table->num_rows - 1;
  while (i < n && table->cells[i][column] == 0) i++;
  if (i == n) return -1;
  const int result = i++;
  while (i < n && table->cells[i][column] == 0) i++;
  return i == n ? result : -1;
}

bool is_basic(const struct table* table, long long int column) {
  return basic_row(table, column) != -1;
}

void canonicalize_table(struct table* table) {
  if (table->num_rows == max_rows ||
      table->num_columns + table->num_rows >= max_columns) {
    die("too big");
  }
  const long long int r = table->num_rows;
  const long long int c = table->num_columns - 1;
  for (long long int i = 0; i < r - 1; i++) {
    if (table->cells[i][c] >= 0) continue;
    for (long long int j = 0; j <= c; j++) table->cells[i][j] = -table->cells[i][j];
  }
  const long long int n = r - 1;
  table->num_rows++;
  table->num_columns += n + 1;
  // Zero all the new space.
  for (long long int i = 0; i < table->num_columns; i++) {
    table->cells[r][i] = 0;
  }
  for (long long int i = 0; i < table->num_rows; i++) {
    for (long long int j = c + 1; j < table->num_columns; j++) {
      table->cells[i][j] = 0;
    }
  }
  // Move the values to the new end.
  for (long long int i = 0; i < r; i++) {
    table->cells[i][c + n + 1] = table->cells[i][c];
    table->cells[i][c] = 0;
  }
  // Prepare the new cost row.
  for (long long int i = 0; i < c - 1; i++) {
    long long int value = 0;
    for (long long int j = 0; j < n; j++) value += table->cells[j][i];
    table->cells[r][i] = value;
  }
  table->cells[r][table->num_columns - 2] = 1;
  // Add artificial variables.
  for (long long int i = 0; i < n; i++) table->cells[i][c + i] = 1;
  for (long long int i = 0; i < n; i++) {
    table->cells[r][table->num_columns - 1] +=
        table->cells[i][table->num_columns - 1];
  }
  simplex_minimize(table);
  const long long int cost = table->cells[table->num_rows - 1][table->num_columns - 1];
  if (cost != 0) die("no feasible solution");
  // Move the basis into the initial set of columns.
  for (int i = 0; i < n; i++) {
    const int b = basic_row(table, c + i);
    if (b == -1) continue;
    // Find a non-basic column to pivot on.
    for (int j = 0; j < c - 1; j++) {
      if (table->cells[b][j] == 0) continue;
      pivot(table, b, j);
      break;
    }
  }
  // Move the rhs back.
  for (long long int i = 0; i < r; i++) {
    table->cells[i][c] = table->cells[i][c + n + 1];
  }
  table->num_rows = r;
  table->num_columns = c + 1;
  for (long long int i = 0; i < r; i++) {
    reduce_row(table, i);
  }
}

long long int non_integer_row(const struct table* table) {
  for (long long int i = 0; i < table->num_columns - 2; i++) {
    if (!is_basic(table, i)) continue;
    for (long long int j = 0; j < table->num_rows - 1; j++) {
      if (table->cells[j][i] == 0) continue;
      const long long int n = table->cells[j][table->num_columns - 1];
      const long long int d = table->cells[j][i];
      if (n % d != 0) return j;
      break;
    }
  }
  return -1;
}

// Finds the column index of the basic variable corresponding to the given row.
long long int basic_variable(const struct table* table, long long int row) {
  for (long long int i = 0; i < table->num_columns - 2; i++) {
    if (table->cells[row][i] != 0 && is_basic(table, i)) return i;
  }
  die("no basic variable");
}

long long int mod(long long int n, long long int d) {
  assert(d > 0);
  return ((n % d) + d) % d;
}

void gomory_cut(struct table* table, long long int row) {
  if (table->num_rows == max_rows || table->num_columns == max_columns) {
    die("too big");
  }
  const long long int column = basic_variable(table, row);
  const long long int d = table->cells[row][column];
  // Insert the new row.
  const long long int r = table->num_rows - 1;
  table->num_rows++;
  for (long long int i = 0; i < table->num_columns; i++) {
    table->cells[r + 1][i] = table->cells[r][i];
    table->cells[r][i] = -mod(table->cells[row][i], d);
  }
  table->cells[r][table->num_columns - 1] =
      -mod(table->cells[row][table->num_columns - 1], d);
  // Move the RHS and the cost variable over by one space.
  const long long int c = table->num_columns - 2;
  table->num_columns++;
  for (long long int i = 0; i < table->num_rows; i++) {
    long long int* out = &table->cells[i][c];
    out[2] = out[1];
    out[1] = out[0];
    out[0] = 0;
  }
  table->cells[r][c] = d;
}

void integer_minimize(struct table* table) {
  while (true) {
    simplex_minimize(table);
    const long long int r = non_integer_row(table);
    if (r == -1) return;
    gomory_cut(table, r);
    dual_simplex(table);
  }
}

struct table table;
long long int joltage_min_presses(const struct machine* machine) {
  build_table(&table, machine);
  canonicalize_table(&table);
  integer_minimize(&table);
  return table.cells[table.num_rows - 1][table.num_columns - 1] /
         table.cells[table.num_rows - 1][table.num_columns - 2];
}

long long int part2() {
  long long int total = 0;
  for (long long int i = 0, n = num_machines; i < n; i++) {
    total += joltage_min_presses(&machines[i]);
  }
  return total;
}

int main() {
  read_input();
  print_uints(part1(), part2());
}

// 33035 too high
// 21772 wrong
// 21782 wrong
