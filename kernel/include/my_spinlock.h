#include <kernel.h>

// write all current CPU cached values to memory
#define barrier() asm volatile("" ::: "memory")

// Pause instruction to prevent excess processor bus usage
#define cpu_relax() asm volatile("pause\n" ::: "memory")

typedef struct {
  volatile unsigned int lock;
} spinlock_t;

// _atomic_xchg() in abstract_machine/am/am.h

void spin_lock(spinlock_t *lk) {
  cpu_relax();
  while (_atomic_xchg(&(lk->lock), 1))
    ;
}

void spin_unlock(spinlock_t *lk) {
  _atomic_xchg(&(lk->lock), 0);
  barrier();
}

/*
// reference: https://blog.csdn.net/k_cnoize/article/details/54946030
// write all current CPU cached values to memory
#define barrier() asm volatile("" ::: "memory")

// Pause instruction to prevent excess processor bus usage
#define cpu_relax() asm volatile("pause\n" ::: "memory")

static inline void *xchg_64(void *ptr, void *x) {
  __asm__ __volatile__("xchgq %0,%1"
                       : "=r"((unsigned long long)x)
                       : "m"(*(volatile long long *)ptr),
                         "0"((unsigned long long)x)
                       : "memory");
  return x;
}

static inline unsigned xchg_32(void *ptr, unsigned x) {
  __asm__ __volatile__("xchgl %0, %1"
                       : "=r"((unsigned)x)
                       : "m"(*(volatile unsigned *)ptr), "0"(x)
                       : "memory");
}

static inline unsigned short xchg_16(void *ptr, unsigned short x) {
  __asm__ __volatile__("xchgw %0,%1"
                       : "=r"((unsigned short)x)
                       : "m"(*(volatile unsigned short *)ptr), "0"(x)
                       : "memory");
  return x;
}

static inline char atomic_bitsetandtest(void *ptr, int x) {
  char out;
  __asm__ __volatile__(
      "lock; bts %2,%1\n"
      "sbb %0,%0\n"
      : "=r"(out), "=m"(*(volatile long long *)ptr)
      : "Ir"(x)
      : "memory");
  return out;
}

// take example as 32 bit
#define EBUSY 1

typedef unsigned spinlock_t;

static void spin_lock(spinlock_t *lock) {
  while (1) {
    if (!xchg_32(lock, EBUSY)) return;
    while (*lock) cpu_relax();
  }
}

static void spin_unlock(spinlock_t *lock) {
  barrier();
  *lock = 0;
}

static int spin_trylock(spinlock_t *lock) { return xchg_32(lock, EBUSY); }

*/