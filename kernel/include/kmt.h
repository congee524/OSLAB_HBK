#include <am.h>
#include <kernel.h>

#define STK_SZ 4096  // the size of stack
#define TASK_MAX 4

struct task tasks;

struct task *current_task[TASK_MAX];

#define current (current_task[_cpu()])