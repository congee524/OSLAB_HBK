#include <am.h>
#include <kernel.h>

void sleep(task_t *chan, spinlock_t *lk) {
  if (!current) panic("sleep");

  if (!lk) panic("sleep without lk");

  if (lk != &sleep_lk) {
    kmt->spin_lock(&sleep_lk);
    kmt->spin_unlock(lk);
  }

  chan->state = SLEEPING;
  _yield();

  if (lk != &sleep_lk) {
    kmt->spin_unlock(&sleep_lk);
    kmt->spin_lock(lk);
  }
}

void wakeupl(task_t *chan) {
  task_t *tmp;
  for (int i = 0; i < _ncpu(); i++) {
    if (tasks[i].cnt > 0) {
      tmp = tasks[i].head;
      if (tmp->status == SLEEPING && strcmp(tmp->name, chan->name) == 0) {
        tmp->status = RUNNABLE;
        flag = 1;
        break;
      }
      while (tmp->next) {
        tmp = tmp->next;
        if (tmp->status == SLEEPING && strcmp(tmp->name, chan->name) == 0) {
          tmp->status = RUNNABLE;
          flag = 1;
          break;
        }
      }
    }
    if (flag) break;
  }
}

void wakeup(task_t *chan) {
  kmt->spin_lock(&sleep_lk);
  wakeupl(chan);
  kmt->spin_unlock(&sleep_lk);
}