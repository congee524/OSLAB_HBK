#include <common.h>
#include <klib.h>

void echo_task(void *name) {
  device_t *tty = dev_lookup(name);
  while (1) {
    char line[128], text[128];
    sprintf(text, "(%s) $ ", name);
    tty_write(tty, text);
    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line);
    tty_write(tty, text);
  }
}

static void os_init() {
  pmm->init();
  kmt->init();
  _vme_init(pmm->alloc, pmm->free);
  dev->init();

  // create thread, able to call tty->ops->read, tty->ops->write
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
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
  if (current) current->context = *ctx;

  do {
    if (!current) {
      current = &tasks;
    } else {
      current = current->next;
    }
  } while ((current - tasks) % _ncpu() != _cpu());
  printf("\n[cpu-%d] Schedule: %s\n", _cpu(), current->name);
  return context;
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
