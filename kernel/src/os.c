#include <common.h>
#include <devices.h>
#include <klib.h>

#define MAX_HANDLER 32

struct {
  int seq;
  int event;
  handler_t handler;
} handlers[MAX_HANDLER];

static void os_init() {
  pmm->init();
  kmt->init();
  _vme_init(pmm->alloc, pmm->free);
  dev->init();
  // vfs->init();
  // create thread, able to call tty->ops->read, tty->ops->write
  /*
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
  */
}

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

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]);
  _putc('\n');
}

static void os_run() {
  hello();
  test();
  _intr_write(1);
  while (1) {
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
  kmt->spin_unlock(&os_trap_lk);
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  // TODO
  kmt->spin_lock(&irq_lk);
  handlers[cnt_handle].seq = seq;
  handlers[cnt_handle].event = event;
  handlers[cnt_handle].handler = handler;
  cnt_handle++;
  // according to seq, call it
  for (int i = cnt_handle - 1; i > 0; i--) {
    if (handlers[cnt_handle].seq < handlers[i - 1].seq) {
      int tmp_seq = handlers[i].seq;
      int tmp_event = handlers[i].event;
      handler_t tmp_handler = handlers[i].handler;
      handlers[i].seq = handlers[i - 1].seq;
      handlers[i].event = handlers[i - 1].event;
      handlers[i].handler = handlers[i - 1].handler;
      handlers[i - 1].seq = tmp_seq;
      handlers[i - 1].event = tmp_event;
      handlers[i - 1].handler = tmp_handler;
    } else {
      break;
    }
  }
  kmt->spin_unlock(&irq_lk);
}

MODULE_DEF(os){
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
