#include <common.h>
#include <klib.h>

static void *s[4][11451] = {};
static void l1_test0() {
  srand(uptime());
  for (int i = 0; i < 11451; ++i) {
    s[_cpu()][i] = pmm->alloc(rand() % (1024));
  }
  for (int i = 0; i < 11451; ++i) {
    pmm->free(s[_cpu()][i]);
  }
  printf("SUCCESS ON CPU %d", _cpu());
}

static void os_init() { pmm->init(); }

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]);
  _putc('\n');
}

static void os_run() {
  hello();
  l1_test0();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) { return context; }

static void os_on_irq(int seq, int event, handler_t handler) {}

MODULE_DEF(os){
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
