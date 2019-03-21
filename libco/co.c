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
static int current;
// static int co_cnt; // to record the num of coroutine
static void *tos; // top of stack


void co_init() {
    current = 0;
    return;
}

struct co* co_start(const char *name, func_t func, void *arg) {
    int pre;
    for (pre = 0; pre < MAXTHREAD; pre++) {
        if (coroutine[pre].used == false) {
            break;
        }
    }
    if (pre == MAXTHREAD) {
        printf("NO ENOUGH THREADS!\n");
        assert(0);
    }

    strcpy(coroutine[pre].name, name);
    // coroutine[co_cnt].name = name;
    coroutine[pre].func = func;
    coroutine[pre].used = true;

    if (tos == NULL) {
        tos = (void *)&arg;
    }
    tos += STACKDIR STACKSIZE;
    char arg_n[STACKDIR(tos - (void *)&arg)];
    coroutine[pre].coarg = arg_n;

    current = pre;

    if (setjmp(coroutine[pre].env)) {
        return (struct co*)(coroutine + pre);
    }


    func(arg); // Test #2 hangs
    return (struct co*)(coroutine + pre);
}

void co_yield() {
    int go;
    for (go = 0; go < MAXTHREAD; go++) {
        if (coroutine[go].used == true) {
            break;
        }
    }
    if (go == MAXTHREAD) {
        printf("NO ACCESSIBLE COROUTINE!\n");
        return;
    }

    if (setjmp(coroutine[current].env)) {
        return;
    }

    current = go;
    longjmp(coroutine[go].env, 1);
}

void co_wait(struct co *thd) {
    memset(thd, 0, sizeof(struct co));
    return;
}

