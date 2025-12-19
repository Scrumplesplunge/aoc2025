#pragma once

#include "io.h"

#define CLONE_VM 0x0000100
#define CLONE_FS 0x0000200
#define CLONE_FILES 0x0000400
#define CLONE_SIGHAND 0x0000800
#define CLONE_THREAD 0x0010000
#define CLONE_SYSVSEM 0x0040000
#define CLONE_CHILD_CLEARTID 0x0200000
#define CLONE_CHILD_SETTID 0x1000000
typedef int pid_t;

long sys_clone(unsigned long flags, void* stack, void* ptid, void* ctid,
               void* tls) {
  ssize_t result;
  asm volatile("int $0x80"
               : "=a"(result)
               : "a"(120), "b"(flags), "c"(stack), "d"(ptid), "S"(tls),
                 "D"(ctid)
               : "memory");
  return result;
}

[[noreturn]] void sys_exit(int code) {
  asm volatile("int $0x80" : : "a"(1), "b"(code));
  __builtin_unreachable();
}

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1

void futex_wait(void* futex, unsigned int value) {
  asm volatile("int $0x80"
               :
               : "a"(240), "b"(futex), "c"(FUTEX_WAIT), "d"(value), "S"(0)
               : "memory");
}

void futex_wake(void* futex, unsigned int num_waiters) {
  asm volatile("int $0x80"
               :
               : "a"(240), "b"(futex), "c"(FUTEX_WAKE), "d"(num_waiters), "S"(0)
               : "memory");
}

enum { thread_stack_size = 1 << 16 };

struct thread {
  pid_t tid;
  void* data;
  void (*func)(void*);
  char stack[thread_stack_size];
};

void start_thread(struct thread* state, void (*f)(void*), void* data) {
  state->tid = -1;
  state->data = data;
  state->func = f;
  // Stash a copy of the state pointer in the new stack.
  char* sp = state->stack + thread_stack_size - sizeof(state);
  memcpy(sp, &state, sizeof(state));
  // We need to guarantee that this is in a register, since once we resume in
  // the new thread we can only rely on rax, rsp, and rip being valid.
  register const pid_t result asm("rax") = sys_clone(
      CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
          CLONE_SYSVSEM | CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID,
      sp, NULL, &state->tid, NULL);
  if (result < 0) die("sys_clone");
  if (result == 0) {
    // We're running in the new thread. We need to recover the state pointer
    // from the copy stashed in the stack.
    asm volatile("mov (%%esp), %0" : "=r"(state));
    state->func(state->data);
    sys_exit(0);
  }
}

void join_thread(struct thread* state) {
  while (true) {
    pid_t t;
    __atomic_load(&state->tid, &t, __ATOMIC_ACQUIRE);
    if (t == 0) break;
    futex_wait(&state->tid, t);
  }
}
