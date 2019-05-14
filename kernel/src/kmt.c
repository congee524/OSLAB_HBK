#include <common.h>
#include <kernel.h>
#include <klib.h>

// spinlock xv6 https://github.com/pelhamnicholas/xv6
// https://github.com/pelhamnicholas/xv6/blob/master/semaphore.c

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)

struct task *current_task[MAX_CPU];

#define current (current_task[_cpu()])

extern void getcallerpcs(void *v, unsigned int pcs[]);
extern int holding(struct spinlock *lock);
extern void pushcli(void);
extern void popcli(void);
extern void sleep(task_t *chan, spinlock_t *lk);
extern void wakeup(task_t *chan);

static inline void panic(const char *s) {
  printf("%s\n", s);
  _halt(1);
}

static void kmt_context_save(_Event ev, _Context *ctx) {
  // TODO
  if (current) current->context = *ctx;
}

static _Context *kmt_context_switch(_Event ev, _Context *ctx) {
  // TODO
  if (!current) {
    current = tasks[_cpu()].head;
  } else {
    current->status = RUNNABLE;
    for (int i = 0; i < tasks[_cpu()].cnt; i++) {
      if (!current->next) {
        current = tasks[_cpu()].head;
      } else {
        current = current->next;
      }
      if (current->status == RUNNABLE) {
        break;
      }
    }
  }
  current->status = RUNNING;
  printf("\n[cpu-%d] Schedule: %s\n", _cpu(), current->name);

  return &current->context;
}

static void kmt_init() {
  // TODO
  // ...
  for (int i = 0; i < _ncpu(); i++) {
    tasks[i].head = NULL;
    tasks[i].cnt = 0;
  }
  kmt->spin_init(&create_lk, "create_lk");
  kmt->spin_init(&teard_lk, "teard_lk");
  kmt->spin_init(&alloc_lk, "alloc_lk");
  kmt->spin_lock(&sleep_lk, "sleep_lk");
  kmt->spin_lock(&irq_lk, "irq_lk");

  os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
  // ...
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg),
                      void *arg) {
  // TODO
  kmt->spin_lock(&create_lk);
  task->name = name;
  task->next = NULL;
  task->status = RUNNABLE;
  _Area stack = (_Area){task->stack, task->fence2};
  task->context = *_kcontext(stack, entry, arg);
  int j = 0;
  for (int i = 1; i < _ncpu(); i++) {
    if (tasks[i].cnt < task[j].cnt) {
      j = i;
    }
  }
  if (!task[j].head) {
    tasks[j].head = task;
  } else {
    task_t *tmp = tasks[j].head;
    while (tmp->next) tmp = tmp->next;
    tmp->next = task;
  }
  tasks[j].cnt++;
  kmt->spin_unlock(&create_lk);
  return 0;
}

static void kmt_teardown(task_t *task) {
  // TODO
  // problem!!!!! if the task in sleeping list
  kmt->spin_lock(&teard_lk);
  int flag = 0;
  task_t *tmp;
  task_t *last;
  int i;
  for (i = 0; i < _ncpu(); i++) {
    if (tasks[i].cnt > 0) {
      last = tmp = tasks[i].head;
      if (strcmp(tmp->name, task->name) == 0) {
        flag = 2;
        break;
      }
      while (tmp->next) {
        last = tmp;
        tmp = tmp->next;
        if (strcmp(tmp->name, task->name) == 0) {
          flag = 1;
          break;
        }
      }
    }
    if (flag) break;
  }

  switch (flag) {
    case 0:
      panic("No such task!");
      break;
    case 1:
      last->next = tmp->next;
      pmm->free(tmp);
      tasks[i].cnt--;
      break;
    case 2:
      tasks[i].head = NULL;
      pmm->free(tmp);
      tasks[i].cnt--;
      break;
    default:
      panic("Wrong flag!");
      break;
  }
  kmt->spin_unlock(&teard_lk);
}

static void kmt_spin_init(spinlock_t *lk, const char *name) {
  strcpy(lk->name, name);
  lk->locked = 0;
  lk->cpu = 0;
}

static void kmt_spin_lock(spinlock_t *lk) {
  pushcli();  // disable interrupts to avoid deadlock.
  if (holding(lk)) panic("acquire");

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
  _atomic_xchg(&(lk->locked), 0);

  popcli();
}

static void kmt_sem_init(sem_t *sem, const char *name, int value) {
  // TODO
  sem->name = name;
  sem->value = value;
  char tmp[128];
  sprintf(tmp, "%s_spinlock", name);
  kmt->spin_init(&sem->lock, tmp);
  end = start = 0;
}

static void kmt_sem_wait(sem_t *sem) {
  // TODO
  kmt->spin_lock(&sem->lock);
  sem.value--;
  if (s.value < 0) {
    s->list[end] = current;
    s->end = (s->end + 1) % NTASK;
    sleep(current, &sem->lock);
  }
  kmt->spin_unlock(&sem->lock);
}

static void kmt_sem_signal(sem_t *sem) {
  // TODO
  kmt->spin_lock(&sem->lock);
  sem.value++;
  if (sem.value <= 0) {
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
    .context_save = kmt_context_save,
    .context_switch = kmt_context_switch,
};
