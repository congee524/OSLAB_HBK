#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <klib.h>
#include <nanos.h>

#define STK_SZ 4096  // the size of stack
#define NTASK 32

#define log(fmt, ...) printf("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)

typedef enum {
  SLEEPING = 0,
  RUNNABLE,
  RUNNING,
  DEAD,
} task_status;

struct task {
  const char *name;
  _Context context;
  int status;
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
  int cpu;               // The cpu holding the lock.
  unsigned int pcs[10];  // The call stack (an array of program counters)
                         // that locked the lock.
};

struct semaphore {
  // TODO
  int value;
  char *name;
  spinlock_t lock;
  task_t *list[NTASK];
  int end;
  int start;
};

#define MAX_CPU 16

struct Task_Pool {
  struct task *head;
  int cnt;
} tasks[MAX_CPU];

// ptable
spinlock_t create_lk;
spinlock_t teard_lk;
spinlock_t irq_lk;
spinlock_t alloc_lk;
spinlock_t os_trap_lk;

struct {
  spinlock_t lock;
  task_t tasks[NTASK];
} ptable;

int8_t ncli[MAX_CPU], intena[MAX_CPU];

#endif
