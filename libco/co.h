#ifndef __CO_H__
#define __CO_H__

// #define MAXTHREAD 10

typedef void (*func_t)(void *arg);
struct co;

/*
co coroutine[MAXTHREAD];
int co_cnt
*/

void co_init();
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);

#endif
