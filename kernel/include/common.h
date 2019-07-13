#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <klib.h>
#include <nanos.h>
#include <vfs.h>

#define STK_SZ 4096 /* the size of stack*/
#define NTASK 32
/*最大文件描述符数量*/
#define NOFILE 32

#define log(fmt, ...) \
  printf("\n%s:%d: \n" fmt, __func__, __LINE__, ##__VA_ARGS__)

enum task_state { RUNNABLE = 0, RUNNING, SLEEPING, DEAD };

struct task {
  char name[128];
  // 文件描述符是基于线程的
  file_t *fildes[NOFILE];
  _Context context;
  int state;
  int cpu;
  void *chan;
  struct task *next;
  struct task *prev;
  uint8_t fence1[32];  // init with 0xcc, check overflow
  uint8_t stack[STK_SZ];
  uint8_t fence2[32];
};

// Mutual exclusion lock.
struct spinlock {
  volatile intptr_t locked;  // Is the lock held?
  // For debugging:
  char name[128];  // Name of lock.
  int cpu;         // The cpu holding the lock.
  // unsigned int pcs[10];  // The call stack (an array of program counters)
  // that locked the lock.
};

struct semaphore {
  // TODO
  spinlock_t lock;
  int value;
  char name[128];
  task_t *list[NTASK];
  int end;
  int start;
};

/*
struct Task_Pool {
  struct task *head;
  int cnt;
} tasks[MAX_CPU];
*/

// ptable
spinlock_t alloc_lk;
spinlock_t os_trap_lk;
spinlock_t print_lk;

int8_t ncli[MAX_CPU], intena[MAX_CPU];

struct {
  spinlock_t lk;
  task_t *tasks;
  int cnt_task;
} ptable;

#endif
