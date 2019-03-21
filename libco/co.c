#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include "co.h"

#define false 0
#define true 1

struct co {
    char name[128];
    func_t func;
    jmp_buf env; // to save the state of present function
    int used; // record the routine whether being used
    void *coarg;
};

#define MAXTHREAD 10
#define STACKDIR - // set - for downwards
#define STACKSIZE (1 << 12)
static struct co coroutine[MAXTHREAD];
static int co_cnt; // to record the num of coroutine
static void *tos; // top of stack


void co_init() {
    co_cnt = 0;
}

struct co* co_start(const char *name, func_t func, void *arg) {
    if (co_cnt >= MAXTHREAD) {
        printf("NO ENOUGH THREADS!\n");
        assert(0);
    }

    strcpy(coroutine[co_cnt].name, name);
    // coroutine[co_cnt].name = name;
    coroutine[co_cnt].func = func;
    coroutine[co_cnt].used = true;

    if (tos == NULL) {
        tos = (void *)&arg;
    }
    tos += STACKDIR STACKSIZE;
    char arg_n[STACKDIR(tos - (void *)&arg)];
    coroutine[co_cnt].coarg = arg_n;

    co_cnt++;
    func(arg); // Test #2 hangs
    return (struct co*)(coroutine + (co_cnt - 1));
}

void co_yield() {
}

void co_wait(struct co *thd) {
    memset()
    co_cnt--;
}

