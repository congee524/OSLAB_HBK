#include <common.h>
#include <kernel.h>
#include <klib.h>
#include <kmt.h>

// spinlock xv6 https://github.com/pelhamnicholas/xv6
// https://github.com/pelhamnicholas/xv6/blob/master/semaphore.c

extern void getcallerpcs(void *v, uint pcs[]);
extern int holding(struct spinlock *lock);
extern void pushcli(void);
extern void popcli(void);

static void kmt_init() {
  // TODO
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg),
                      void *arg) {
  // TODO
  return 0;
}

static void kmt_teardown(task_t *task) {
  // TODO
}

static void kmt_spin_init(spinlock_t *lk, const char *name) {
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

static void kmt_spin_lock(spinlock_t *lk) {
  pushcli();  // disable interrupts to avoid deadlock.
  if (holding(lk)) panic("acquire");

  // The xchg is atomic.
  // It also serializes, so that reads after acquire are not
  // reordered before it.
  while (xchg(&lk->locked, 1) != 0)
    ;

  // Record info about lock acquisition for debugging.
  lk->cpu = cpu;
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
static void kmt_spin_unlock(spinlock_t *lk) {
  if (!holding(lk)) panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // The xchg serializes, so that reads before release are
  // not reordered after it.  The 1996 PentiumPro manual (Volume 3,
  // 7.2) says reads can be carried out speculatively and in
  // any order, which implies we need to serialize here.
  // But the 2007 Intel 64 Architecture Memory Ordering White
  // Paper says that Intel 64 and IA-32 will not move a load
  // after a store. So lock->locked = 0 would work here.
  // The xchg being asm volatile ensures gcc emits it after
  // the above assignments (and after the critical section).
  xchg(&lk->locked, 0);

  popcli();
}

static void kmt_sem_init(sem_t *sem, const char *name, int value) {
  // TODO
  sem->val = value;
  // cprintf("%s sem_init: val = %d\n", name, s->val);
  kmt_spin_init(&sem->lock, (char *)sem);
  for (int j = 0; j < NPROC; j++) sem->thread[j] = 0;
  sem->next = sem->end = 0;
}

static void kmt_sem_wait(sem_t *sem) {
  // TODO
  kmt_spin_lock(&sem->lock);
  // s->thread[s->end] = proc;
  // s->end = (s->end + 1) % NPROC;
  while (sem->val == 0) {
    sem->thread[sem->end] = proc;
    sem->end = (sem->end + 1) % NPROC;
    sleep(proc, &sem->lock);
  }
  sem->val = sem->val - 1;
  // cprintf("%s sem_wait: val = %d\n", s->name, s->val);
  kmt_spin_unlock(&sem->lock);
}

static void kmt_sem_signal(sem_t *sem) {
  // TODO
  kmt_spin_lock(&sem->lock);
  sem->val = sem->val + 1;
  // cprintf("%s sem_signal: val = %d\n", s->name, s->val);
  if (sem->thread[sem->next]) {
    wakeup(sem->thread[sem->next]);
    sem->thread[sem->next] = 0;
    sem->next = (sem->next + 1) % NPROC;
  }
  kmt_spin_unlock(&sem->lock);
}

static void kmt_sem_broadcast(sem_t *sem) {
  // wakeup all the waiting proc
  kmt_spin_lock(&sem->lock);
  sem->val = sem->val + 1;
  // cprintf("%s sem_signal: val = %d\n", s->name, s->val);
  sem->next = (sem->next + 1) % NPROC;
  for (int i = 0; i < NPROC; i++) wakeup(sem->thread[i]);
  kmt_spin_unlock(&sem->lock);
}

MODULE_DEF(kmt){
    .init = kmt_init,
    .create = kmt_create,
    .teardown = kmt_teardown,
    .spin_init = kmt_spin_init,
    .spin_lock = kmt_spin_lock,
    .spin_unlock = kmt_spin_unlock,
    .sem_init = kmt_sem_init,
    .sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal,
    .sem_broadcast = kmt_sem_broadcast,
    //.context_save = kmt_context_save,
    //.context_switch = kmt_context_switch,
};
