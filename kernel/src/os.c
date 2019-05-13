#include <common.h>
#include <devices.h>
#include <klib.h>

#define current (current_task[_cpu()])

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
  for_each(handler in handlers) {
    if (handler->event == _EVENT_NULL || handler->event == ev.event) {
      _Context *next = handler->handler(ev, context);
      if (next) ret = next;
    }
  }
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  // TODO
}

MODULE_DEF(os){
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
