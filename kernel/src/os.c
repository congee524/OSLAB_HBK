#include <common.h>
#include <devices.h>
#include <klib.h>

#define MAX_HANDLER 32

struct {
  int seq;
  _Event event;
  handler_t handler;
} handlers[MAX_HANDLER];

static void os_init() {
  pmm->init();
  kmt->init();
  // _vme_init(pmm->alloc, pmm->free);
  dev->init();
  vfs->init();
  // create thread, able to call tty->ops->read, tty->ops->write
  /*
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
  */
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
  // l1_test1();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *ctx) {
  _Context *ret = NULL;
  for (int i = 0; i < MAX_HANDLER; i++) {
    if (handlers[i].event == _EVENT_NULL || handlers[i].event == ev.event) {
      _Context *next = handler->handler(ev, context);
      if (next) ret = next;
    }
  }
  return ret;
}

int cnt_handle = 0;
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
      _Event tmp_event = handlers[i].event;
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
