#include <common.h>
#include <devices.h>
#include <klib.h>

#define MAX_HANDLER 32

struct irq_hand {
  int seq;
  int event;
  handler_t handler;
} handlers[MAX_HANDLER];

static void os_init() {
  pmm->init();
  kmt->init();
  //_vme_init(pmm->alloc, pmm->free);
  dev->init();
  // vfs->init();
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
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]);
  _putc('\n');
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
    _yield();
  }
}

int cnt_handle = 0;
static _Context *os_trap(_Event ev, _Context *ctx) {
  // kmt->spin_lock(&os_trap_lk);
  _Context *ret = NULL;
  for (int i = 0; i < cnt_handle; i++) {
    if (handlers[i].event == _EVENT_NULL || handlers[i].event == ev.event) {
      _Context *next = handlers[i].handler(ev, ctx);
      if (next) ret = next;
    }
  }
  assert(ret);
  // kmt->spin_unlock(&os_trap_lk);
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  // TODO
  // kmt->spin_lock(&os_trap_lk);
  handlers[cnt_handle].seq = seq;
  handlers[cnt_handle].event = event;
  handlers[cnt_handle].handler = handler;
  cnt_handle++;

  // according to seq, call it
  for (int i = cnt_handle - 1; i > 0; i--) {
    if (handlers[cnt_handle].seq < handlers[i - 1].seq) {
      struct irq_hand tmp = handlers[i];
      handlers[i] = handlers[i - 1];
      handlers[i - 1] = tmp;
    } else {
      break;
    }
  }
  printf("\nirq: ");
  for (int i = 0; i < cnt_handle; i++) {
    printf("%d %d | ", handlers[i].seq, handlers[i].event);
  }
  printf("\n");
  // kmt->spin_unlock(&os_trap_lk);
}

MODULE_DEF(os){
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
