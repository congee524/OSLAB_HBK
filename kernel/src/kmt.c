#include <common.h>
#include <kernel.h>
#include <klib.h>
// spinlock xv6 https://github.com/pelhamnicholas/xv6
// https://github.com/pelhamnicholas/xv6/blob/master/semaphore.c

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX)  // printf cannot printf INT_MIN
typedef unsigned int uint;

task_t *current_task[MAX_CPU];

#define current (current_task[_cpu()])

static inline void panic(const char *s) {
  printf("%s\n", s);
  _halt(1);
}

//==========================================
//==========================================
// handler

static _Context *kmt_context_save(_Event ev, _Context *ctx) {
  // TODO
  kmt->spin_lock(&ptable.lk);
  if (current) current->context = *ctx;
  kmt->spin_unlock(&ptable.lk);
  return &current->context;
}

static _Context *kmt_context_switch(_Event ev, _Context *ctx) {
  // TODO
  kmt->spin_lock(&ptable.lk);
  if (!current) {
    /*
    assert(tasks[_cpu()].head);
    current = tasks[_cpu()].head;
    */
    for (task_t *tmp = ptable.tasks; tmp->next != ptable.tasks;
         tmp = tmp->next) {
      // log("111");
      if (tmp->cpu == _cpu() && tmp->status == RUNNABLE) {
        current = tmp;
        current->status = RUNNING;
        break;
      }
    }
    if (!current) panic("no task to switch!");
  } else {
    task_t *tmp = current;
    do {
      tmp = tmp->next;
      if (tmp == current) {
        log("switch failure\n");
        break;
      }
      // log("222");
    } while (tmp->cpu != _cpu() || tmp->status != RUNNABLE);
    if (current != tmp) {
      if (current->status == RUNNING) current->status = RUNNABLE;
      current = tmp;
      current->status = RUNNING;
    }
  }
  printf("\n[cpu-%d] Schedule: %s\n", _cpu(), current->name);
  task_t *ppp = ptable.tasks;
  while (ppp->next != ptable.tasks) {
    ppp = ppp->next;
    printf("name: %s status: %d cpu: %d\n", ppp->name, ppp->status, ppp->cpu);
  }
  printf("\n");
  kmt->spin_unlock(&ptable.lk);
  return &current->context;
}

//==========================================
//==========================================
// task schedule init create teardown
static void kmt_init() {
  // TODO
  // ...
  /*
  for (int i = 0; i < _ncpu(); i++) {
    tasks[i].head = NULL;
    tasks[i].cnt = 0;
  }s
  */
  ptable.tasks = pmm->alloc(sizeof(task_t));
  ptable.tasks->prev = ptable.tasks->next = ptable.tasks;
  ptable.tasks->cpu = -1;
  // ptable.tasks = NULL;
  ptable.cnt_task = 0;
  memset(ncli, 0, sizeof(ncli));
  memset(intena, 0, sizeof(intena));
  kmt->spin_init(&alloc_lk, "alloc_lk");
  kmt->spin_init(&os_trap_lk, "os_trap_lk");
  kmt->spin_init(&ptable.lk, "ptable.lk");

  os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
  // ...
}

static int task_insert(task_t *task) {
  // insert from head
  if (!task) {
    log("task is empty!");
    return 1;
  }
  if (!ptable.tasks) {
    log("ptable.tasks init failure!");
    return 1;
  }
  task->next = ptable.tasks->next;
  ptable.tasks->next->prev = task;
  ptable.tasks->next = task;
  task->prev = ptable.tasks;
  return 0;
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg),
                      void *arg) {
  // TODO
  kmt->spin_lock(&ptable.lk);
  strcpy(task->name, name);
  task->cpu = ptable.cnt_task % _ncpu();
  task->status = RUNNABLE;
  _Area stack = (_Area){task->stack, task->fence2};
  task->context = *_kcontext(stack, entry, arg);

  if (task_insert(task)) panic("list insert wrong!");

  ptable.cnt_task++;
  kmt->spin_unlock(&ptable.lk);
  return 0;
}

static void kmt_teardown(task_t *task) {
  // TODO
  // problem!!!!! if the task in sleeping list
  return;
}

//==========================================
//==========================================
// spin_lock

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000  // First kernel virtual address
#define FL_IF 0x00000200     // Interrupt Enable

static inline void sti(void) { asm volatile("sti"); }

static inline uint readeflags(void) {
  uint eflags;
  asm volatile("pushfl; popl %0" : "=r"(eflags));
  return eflags;
}

static inline void cli(void) { asm volatile("cli"); }

// Record the current call stack in pcs[] by following the %ebp chain.
void getcallerpcs(void *v, uint pcs[]) {
  uint *ebp;
  int i;

  ebp = (uint *)v - 2;
  for (i = 0; i < 10; i++) {
    if (ebp == 0 || ebp < (uint *)KERNBASE || ebp == (uint *)0xffffffff) break;
    pcs[i] = ebp[1];       // saved %eip
    ebp = (uint *)ebp[0];  // saved %ebp
  }
  for (; i < 10; i++) pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int holding(spinlock_t *lock) { return lock->locked && lock->cpu == _cpu(); }

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off
void pushcli(void) {
  uint eflags;
  eflags = readeflags();
  cli();
  if (ncli[_cpu()]++ == 0) intena[_cpu()] = eflags & FL_IF;
  // if (ncli[_cpu()]++ == 0) intena[_cpu()] = _intr_read();
}

void popcli(void) {
  if (readeflags() & FL_IF) panic("popcli - interruptible");
  if (--ncli[_cpu()] < 0) panic("popcli");
  if (ncli[_cpu()] == 0 && intena[_cpu()]) sti();
}

static void kmt_spin_init(spinlock_t *lk, const char *name) {
  strcpy(lk->name, name);
  lk->locked = 0;
  lk->cpu = -1;
}

static void kmt_spin_lock(spinlock_t *lk) {
  pushcli();  // disable interrupts to avoid deadlock.
  if (holding(lk)) {
    log("%s\n", lk->name);
    panic("acquire");
  }
  // The xchg is atomic.
  // It also serializes, so that reads after acquire are not
  // reordered before it.
  while (_atomic_xchg(&(lk->locked), 1) != 0)
    ;

  // Record info about lock acquisition for debugging.
  lk->cpu = _cpu();
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
static void kmt_spin_unlock(spinlock_t *lk) {
  if (!holding(lk)) {
    log("%s\n", lk->name);
    panic("release");
  }

  lk->pcs[0] = 0;
  lk->cpu = -1;

  // The xchg serializes, so that reads before release are
  // not reordered after it.  The 1996 PentiumPro manual (Volume 3,
  // 7.2) says reads can be carried out speculatively and in
  // any order, which implies we need to serialize here.
  // But the 2007 Intel 64 Architecture Memory Ordering White
  // Paper says that Intel 64 and IA-32 will not move a load
  // after a store. So lock->locked = 0 would work here.
  // The xchg being asm volatile ensures gcc emits it after
  // the above assignments (and after the critical section).
  _atomic_xchg(&(lk->locked), 0);

  popcli();
}

//==========================================
//==========================================
// semaphore

void sleep(task_t *chan, spinlock_t *lk) {
  log("sleep name %s, status %d\n", chan->name, chan->status);
  if (!current) panic("sleep");
  if (!lk) panic("sleep without lk");
  task_t *t = current;
  t->chan = chan;
  t->status = SLEEPING;
  kmt->spin_unlock(lk);
  _yield();
  kmt->spin_lock(lk);
  t->chan = 0;
}

void wakeup(task_t *chan) {
  kmt->spin_lock(&ptable.lk);
  log("!!!!!!to be wake name: %s, status: %d", chan->name, chan->status);
  task_t *tmp;
  for (tmp = ptable.tasks->next; tmp != ptable.tasks; tmp = tmp->next) {
    printf("wakeing name: %s, status: %d\n", tmp->name, tmp->status);
    if (tmp->chan) printf(" chan: %s\n", ((task_t *)tmp->chan)->name);
    if (tmp->status == SLEEPING && tmp->chan == chan) {
      tmp->status = RUNNABLE;
    }
  }
  kmt->spin_unlock(&ptable.lk);
}

static void kmt_sem_init(sem_t *sem, const char *name, int value) {
  // TODO
  strcpy(sem->name, name);
  sem->value = value;
  char tmp[128];
  sprintf(tmp, "%s_lk", name);
  kmt->spin_init(&sem->lock, tmp);
  sem->end = sem->start = 0;
}

static void kmt_sem_wait(sem_t *sem) {
  // TODO
  kmt->spin_lock(&sem->lock);
  // log("\nkmt spin lock %s\nsem_value %d\n", sem->name, sem->value);
  // log("wait value b %d\n", sem->value);
  sem->value--;
  // log("wait value a %d\n", sem->value);
  if (sem->value < 0) {
    sem->list[sem->end] = current;
    sem->end = (sem->end + 1) % NTASK;
    sleep(current, &sem->lock);
  }
  kmt->spin_unlock(&sem->lock);
}

static void kmt_sem_signal(sem_t *sem) {
  // TODO
  kmt->spin_lock(&sem->lock);
  // log("\nkmt spin unlock %s\nsem_value %d\n", sem->name, sem->value);
  // log("signal value b %d\n", sem->value);
  sem->value++;
  // log("signal value a %d\n", sem->value);
  if (sem->value <= 0) {
    wakeup(sem->list[sem->start]);
    sem->list[sem->start] = NULL;
    sem->start = (sem->start + 1) % NTASK;
  }
  kmt->spin_unlock(&sem->lock);
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
};
