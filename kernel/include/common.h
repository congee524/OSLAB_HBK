#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <klib.h>
#include <nanos.h>

#define STK_SZ 4096  // the size of stack
#define NPROC 32

struct task {
  const char *name;
  _Context context;
  struct task *next;
  uint8_t fence1[32];  // init with 0xcc, check overflow
  uint8_t stack[STK_SZ];
  uint8_t fence2[32];
};

// Mutual exclusion lock.
struct spinlock {
  volatile intptr_t locked;  // Is the lock held?

  // For debugging:
  char *name;            // Name of lock.
  struct cpu *cpu;       // The cpu holding the lock.
  unsigned int pcs[10];  // The call stack (an array of program counters)
                         // that locked the lock.
};
struct semaphore {
  // TODO
};

#endif
