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
#define MAXNAMELEN 128

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX)
/*printf cannot printf INT_MIN */
#define MAX_CPU 16

/*========== flags =========*/
#define O_ACCMODE 0x00000003
/*只读 */
#define O_RDONLY 0x00000000
/*只写 */
#define O_WRONLY 0x00000001
/*可读可写 */
#define O_RDWR 0x00000002
/*创建并打开，如原文件存在则删除 */
#define O_CREAT 0x00000040
/*确保该调用创建文件，如果文件已经存在，返回错误EEXIST */
#define O_EXCL 0x00000080
#define O_NOCTTY 0x00000100
/*原文件内容舍弃 */
#define O_TRUNC 0x00000200
/*新写入内容在原文件后 */
#define O_APPEND 0x00000400
/*对设备文件，改为非阻塞 */
#define O_NONBLOCK 0x00000800
/*不经过缓冲取，立刻flush */
#define O_DSYNC 0x00001000
#define FASYNC 0x00002000
/*如果是符号链接，返回错误ELOOP */
#define O_NOFOLLOW 0x00020000
/*========== flags =========*/
typedef unsigned int uint;

#define log(fmt, ...) \
  printf("\n%s:%d: \n" fmt, __func__, __LINE__, ##__VA_ARGS__)

enum task_state { RUNNABLE = 0, RUNNING, SLEEPING, DEAD };

#define MAXPATHLEN 512
#define MAXDIRITEM 32

struct task {
  char name[MAXNAMELEN];
  // 文件描述符是基于线程的
  file_t *fildes[NOFILE];
  char *pwd;
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
  char name[MAXNAMELEN];  // Name of lock.
  int cpu;                // The cpu holding the lock.
  // unsigned int pcs[10];  // The call stack (an array of program counters)
  // that locked the lock.
};

struct semaphore {
  // TODO
  spinlock_t lock;
  int value;
  char name[MAXNAMELEN];
  task_t *list[NTASK];
  int end;
  int start;
};

int8_t ncli[MAX_CPU], intena[MAX_CPU];

task_t *current_task[MAX_CPU];
#define cur_task (current_task[_cpu()])

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

struct {
  spinlock_t lk;
  task_t *tasks;
  int cnt_task;
} ptable;

#endif
