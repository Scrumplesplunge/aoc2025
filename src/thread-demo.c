#include "core/io.h"
#include "core/threads.h"

enum { num_threads = 4 };
struct thread threads[num_threads];

typedef unsigned int uint32;
typedef unsigned long long uint64;

struct task { uint32 begin, end, result; };
void run(void* data) {
  struct task* task = data;
  uint32 result = 0;
  for (uint32 i = task->begin; i < task->end; i++) result ^= i * i;
  task->result = result;
}

int main() {
  struct task tasks[num_threads];
  enum { max_value = 1'000'000'000 };
  for (uint32 i = 0; i < num_threads; i++) {
    tasks[i] = (struct task){
        .begin = (uint64)max_value * i / num_threads,
        .end = (uint64)max_value * (i + 1) / num_threads,
    };
    start_thread(&threads[i], run, &tasks[i]);
  }
  uint32 total = 0;
  for (int i = 0; i < num_threads; i++) {
    join_thread(&threads[i]);
    total ^= tasks[i].result;
  }

  print_uint(total);
}
