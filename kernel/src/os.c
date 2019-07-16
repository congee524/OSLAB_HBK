#include <common.h>
#include <devices.h>
#include <klib.h>
#include <shell.h>

#define MAX_HANDLER 32

#define ECHO_TASK

struct irq_hand {
  int seq;
  int event;
  handler_t handler;
} handlers[MAX_HANDLER];

void idle(void *arg) {
  while (1) {
    _yield();
  }
}
#ifdef ECHO_TASK
/*
static void echo_task(void *arg) {
  char *name = (char *)arg;
  char line[128] = "", text[128] = "";
  device_t *tty = dev_lookup(name);
  while (1) {
    sprintf(text, "(%s) $ ", name);
    tty->ops->write(tty, 0, text, strlen(text));
    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line);
    tty->ops->write(tty, 0, text, strlen(text));
  }
}

static void create_threads() {
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
}
*/
static void create_threads() {
  kmt->spin_lock(&print_lk);
  log("cur_task->pwd: %s\n", cur_task->pwd);
  kmt->spin_unlock(&print_lk);
  kmt->create(pmm->alloc(sizeof(task_t)), "print", shell_thread, "1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", shell_thread, "2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", shell_thread, "3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", shell_thread, "4");
  kmt->spin_lock(&print_lk);
  log("cur_task->pwd: %s\n", cur_task->pwd);
  kmt->spin_unlock(&print_lk);
}
#endif

static void os_init() {
  pmm->init();
  kmt->init();
  dev->init();
  vfs->init();
  kmt->spin_lock(&print_lk);
  log("cur_task->pwd: %s\n", cur_task->pwd);
  kmt->spin_unlock(&print_lk);
  for (int i = 0; i < _ncpu(); i++) {
    kmt->create(pmm->alloc(sizeof(task_t)), "idle", idle, 0);
  }
  kmt->spin_lock(&print_lk);
  log("cur_task->pwd: %s\n", cur_task->pwd);
  kmt->spin_unlock(&print_lk);
#ifdef ECHO_TASK
  create_threads();
#endif
}
/*
void test() {
  void *space[100];
  int i;
  for (i = 0; i < 100; ++i) {
    space[i] = pmm->alloc(rand() % ((1 << 10) - 1));
  }
  for (i = 0; i < 1000; ++i) {
    int temp = rand() % 10;
    pmm->free(space[temp]);
    space[temp] = pmm->alloc(rand() & ((1 << 10) - 1));
  }
  for (i = 0; i < 100; ++i) {
    pmm->free(space[i]);
  }
}
*/
static void hello() {
  kmt->spin_lock(&print_lk);
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]);
  _putc('\n');
  kmt->spin_unlock(&print_lk);
  kmt->spin_lock(&print_lk);
  log("cur_task->pwd: %s\n", cur_task->pwd);
  kmt->spin_unlock(&print_lk);
}

static void os_run() {
  hello();
  // test();
  /*
    while (_cpu() != 0) {
      while (1)
        ;
    }
  */
  _intr_write(1);
  while (1) {
    kmt->spin_lock(&print_lk);
    log("cur_task->pwd: %s\n", cur_task->pwd);
    kmt->spin_unlock(&print_lk);
    _yield();
  }
}

int cnt_handle = 0;
static _Context *os_trap(_Event ev, _Context *ctx) {
  kmt->spin_lock(&os_trap_lk);
  _Context *ret = NULL;
  for (int i = 0; i < cnt_handle; i++) {
    if (handlers[i].event == _EVENT_NULL || handlers[i].event == ev.event) {
      _Context *next = handlers[i].handler(ev, ctx);
      if (next) ret = next;
    }
  }
  assert(ret);
  kmt->spin_unlock(&os_trap_lk);
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  // TODO
  kmt->spin_lock(&os_trap_lk);
  handlers[cnt_handle].seq = seq;
  handlers[cnt_handle].event = event;
  handlers[cnt_handle].handler = handler;
  cnt_handle++;

  // according to seq, call it
  for (int i = cnt_handle - 1; i > 0; i--) {
    if (handlers[i].seq < handlers[i - 1].seq) {
      struct irq_hand tmp = handlers[i];
      handlers[i] = handlers[i - 1];
      handlers[i - 1] = tmp;
    } else {
      break;
    }
  }
  /*
  printf("\nirq: ");
  for (int i = 0; i < cnt_handle; i++) {
    printf("%d %d | ", handlers[i].seq, handlers[i].event);
  }
  */
  kmt->spin_unlock(&os_trap_lk);
}

MODULE_DEF(os){
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
