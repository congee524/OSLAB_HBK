#ifndef __KMT_H__
#define __KMT_H__
#include <common.h>

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX)
/*printf cannot printf INT_MIN */
#define MAX_CPU 16
typedef unsigned int uint;

task_t *current_task[MAX_CPU];

#define current (current_task[_cpu()])

#endif