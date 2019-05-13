#include <am.h>
#include <kernel.h>

#define STK_SZ 4096  // the size of stack
#define TASK_MAX 4

struct task {
  const char *name;
  _Context context;
  struct task *next;
  uint8_t fence1[32];  // init with 0xcc, check overflow
  uint8_t stack[STK_SZ];
  uint8_t fence2[32];
};

struct task tasks;

struct task *current_task[TASK_MAX];

#define current (current_task[_cpu()])