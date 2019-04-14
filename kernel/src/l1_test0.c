#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void* s[4][11451] = {};
static void l1_test0() {
  srand((unsigned int)time(NULL));
  for (int i = 0; i < 11451; ++i) {
    s[_cpu()][i] = pmm->alloc(rand() % (1024));
  }
  for (int i = 0; i < 11451; ++i) {
    pmm->free(s[_cpu()][i]);
  }

  printf("SUCCESS ON CPU %d", _cpu());
}