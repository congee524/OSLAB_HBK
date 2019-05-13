#include <am.h>
#include <kernel.h>

#define TASK_MAX 4

struct task tasks;

struct task *current_task[TASK_MAX];

#define current (current_task[_cpu()])