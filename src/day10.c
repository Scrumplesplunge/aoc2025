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
int num_machines;
struct machine machines[max_machines];

void read_input() {
  enum { buffer_size = 21 << 10 };
  char buffer[buffer_size];
  const int n = read(STDIN_FILENO, buffer, buffer_size);
  if (n == 0 || buffer[n - 1] != '\n') die("bad input len");

  const char* i = buffer;
  const char* end = i + n;

  while (i != end) {
    if (num_machines == max_machines) die("bad input machine count");
    struct machine* const m = &machines[num_machines++];
    if (*i++ != '[') die("bad input");
    // Read the target light sequence.
    unsigned int next_bit = 1;
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

int popcount(unsigned int x) {
  int total;
  asm("popcnt %1, %0" : "=r"(total) : "r"(x));
  return total;
}

int part1() {
  int total = 0;
  for (int i = 0, n = num_machines; i < n; i++) {
    const struct machine* m = &machines[i];
    int best_count = max_buttons + 1;
    for (int set = 0, all = (1 << m->num_buttons) - 1; set < all; set++) {
      uint16 result = 0;
      for (int i = 0; i < max_buttons; i++) {
        if (set & (1 << i)) result ^= m->buttons[i];
      }
      if (result != m->target) continue;
      const int count = popcount(set);
      if (count >= best_count) continue;
      best_count = count;
    }
    total += best_count;
  }
  return total;
}

enum { max_rows = 128, max_columns = 128 };
struct table {
  // Number of rows in the Tableau.
  int num_rows;
  // Number of columns **including the RHS**
  int num_columns;
  // Simplex Tableau:
  // | 1 0 0 0  -t^T  t^T  | 0 |
  // | 0 1 0 0  b1    -b1  | 1 |
  // | 0 0 1 0  ..     ..  | 1 |
  // | 0 0 0 1  bN    -bN  | 1 |
  int cells[max_rows][max_columns];
};

void swap(int* a, int* b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void swap_rows(struct table* t, int a, int b) {
  for (int i = 0, c = t->num_columns; i < c; i++) {
    swap(&t->cells[a][i], &t->cells[b][i]);
  }
}

int abs(int x) { return x < 0 ? -x : x; }

int gcd(int a, int b) {
  while (b != 0) {
    const int temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

void reduce_row(struct table* t, int row) {
  int divisor = 0;
  const int n = t->num_columns;
  for (int i = 0; i < n; i++) {
    const int v = abs(t->cells[row][i]);
    if (v == 0) continue;
    if (divisor == 0) {
      divisor = v;
    } else {
      divisor = gcd(divisor, v);
    }
  }
  if (divisor == 0) return;
  for (int i = 0; i < n; i++) t->cells[row][i] /= divisor;
}

int lcm(int a, int b) { return a * b / gcd(a, b); }

void print_table(const struct table* table) {
  for (int r = 0; r < table->num_rows; r++) {
    for (int c = 0; c < table->num_columns; c++) {
      print("\t%d", table->cells[r][c]);
    }
    print("\n");
  }
}

int pivot_column(const struct table* table) {
  const int* cost_row = table->cells[0];
  const int n = table->num_columns;
  int best = -1;
  int best_value = 0;
  for (int i = 1; i < n; i++) {
    if (cost_row[i] < best_value) {
      best = i;
      best_value = cost_row[i];
    }
  }
  return best;
}

int pivot_row(const struct table* table, int column) {
  int best = -1;
  int best_n = 0, best_d = 1;
  for (int i = 1, r = table->num_rows; i < r; i++) {
    const int d = table->cells[i][column];
    if (d <= 0) continue;
    const int n = table->cells[i][table->num_columns - 1];
    if (best == -1 || best_d * n < best_n * d) {
      best = i;
      best_n = n;
      best_d = d;
    }
  }
  return best;
}

void simplex_maximize(struct table* table) {
  while (true) {
    const int c = pivot_column(table);
    if (c == -1) break;
    const int r = pivot_row(table, c);
    if (r == -1) die("unsolvable");
    const int* pivot_row = table->cells[r];
    const int pivot_factor = pivot_row[c];
    // print("pivot on column %d, row %d:\n", c, r);
    for (int i = 0; i < table->num_rows; i++) {
      if (i == r) continue;
      int* row = table->cells[i];
      const int row_factor = row[c];
      if (row_factor == 0) continue;
      // print("row %d has factor %d\n", i, row_factor);
      for (int j = 0; j < table->num_columns; j++) {
        const int result = pivot_factor * row[j] - row_factor * pivot_row[j];
        // if (j == c || j == table.num_columns - 1)
        //   print("row[%d] = %d * %d - %d * %d = %d\n",
        //       j, pivot_factor, row[j], row_factor, pivot_row[j], result);
        row[j] = result;
      }
      assert(row[table->num_columns - 1] >= 0);
      assert(row[c] == 0);
      reduce_row(table, i);
    }
  }
}

int mod(int a, int b) { return ((a % b) + b) % b; }

int part2() {
  int total = 0;
  for (int i = 0, n = num_machines; i < n; i++) {
    // Build the matrix.
    const struct machine* machine = &machines[i];
    struct table table = {
        .num_rows = machine->num_buttons + 1,
        .num_columns = 2 * machine->size + machine->num_buttons + 2};
    // Build the Simplex Tableau
    {
      int* row = table.cells[0];
      row[0] = 1;
      for (int i = 1; i <= machine->num_buttons; i++) row[i] = 0;
      for (int i = 0; i < machine->size; i++) {
        const int t = machine->joltages[i];
        row[machine->num_buttons + 1 + i] = -t;
        row[machine->num_buttons + 1 + i + machine->size] = t;
      }
      row[table.num_columns - 1] = 0;
    }
    for (int b = 0; b < machine->num_buttons; b++) {
      int* row = table.cells[b + 1];
      row[0] = 0;
      for (int i = 1; i <= machine->num_buttons; i++) row[i] = 0;
      row[1 + b] = 1;
      for (int i = 0; i < machine->size; i++) {
        const int x = (machine->buttons[b] >> i) & 1;
        row[machine->num_buttons + 1 + i] = x;
        row[machine->num_buttons + 1 + i + machine->size] = -x;
      }
      row[table.num_columns - 1] = 1;
    }
    print("initial:\n");
    print_table(&table);
    // Reduce the table.
    while (true) {
      simplex_maximize(&table);
      print("optimized:\n");
      print_table(&table);
      // Find a non-integer primal variable.
      int non_int = -1;
      for (int i = 1; i <= machine->num_buttons; i++) {
        if (table.cells[0][i] % table.cells[0][0] != 0) {
          non_int = i;
          break;
        }
      }
      if (non_int == -1) break;
      // Solution is not integer. Perform a Gomory cut.
      if (table.num_rows == max_rows || table.num_columns == max_columns) {
        die("too big");
      }
      table.num_columns++;
      const int c = table.num_columns - 2;
      for (int r = 0; r < table.num_rows; r++) {
        table.cells[r][c + 1] = table.cells[r][c];
      }
      table.cells[0][c] = -(table.cells[0][non_int] % table.cells[0][0]);
      for (int r = 1; r < table.num_rows; r++) {
        table.cells[r][c] = -mod(table.cells[r][non_int], table.cells[0][0]);
      }
      for (int i = 0; i < table.num_columns; i++) {
        table.cells[table.num_rows][i] = 0;
      }
      table.cells[table.num_rows][table.num_columns - 2] = 1;
      table.num_rows++;
      print("tweaked:\n");
      print_table(&table);
    }
    print("optimal:\n");
    print_table(&table);
    // Check our answer.
    int counters[max_size] = {};
    const int* answer = table.cells[0] + 1;
    print("\n");
    for (int i = 0; i < machine->num_buttons; i++) {
      print("button[%d]", i);
      const uint16 b = machine->buttons[i];
      const int k = answer[i] / table.cells[0][0];
      for (int j = 0; j < machine->size; j++) {
        print("\t%d", (b >> j) & 1);
        if ((b >> j) & 1) counters[j] += k;
      }
      print("\n");
    }
    print("expected ");
    for (int i = 0; i < machine->size; i++) {
      print("\t%d", machine->joltages[i]);
    }
    print("\nactual   ");
    for (int i = 0; i < machine->size; i++) {
      print("\t%d", counters[i]);
    }
    print("\ndiff     ");
    for (int i = 0; i < machine->size; i++) {
      print("\t%d", machine->joltages[i] - counters[i]);
    }
    print("\n");
    const int num = table.cells[0][table.num_columns - 1];
    const int den = table.cells[0][0];
    total += num / den;
  }
  return total;
}

int main() {
  read_input();
  print_uints(part1(), part2());
}

// 33035 too high
// 21772 wrong
