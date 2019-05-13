#include <am.h>
#include <kernel.h>

typedef struct semaphore {
  unsigned int val;
  struct spinlock lock;
  void* thread[NPROC];
  unsigned int next;
  unsigned int end;
} semaphore;