#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>

static void* s[4][11451] = {};
static void test() {
  srand(uptime());
  for (int i = 0; i < 11451; ++i) {
    s[_cpu()][i] = pmm->alloc(rand() % (1024));
  }
  for (int i = 0; i < 11451; ++i) {
    pmm->free(s[_cpu()][i]);
  }

  printf("SUCCESS ON CPU %d", _cpu());
}