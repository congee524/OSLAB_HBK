#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include "co.h"

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct co {
    char name[128];
    func_t func;
    jmp_buf buf; // to save the state of present function
    int state; // record the state of routine
    void *coarg;
    uint8_t stack[4096];
};

#define MAX_CO 10
#define STACKDIR - // set - for downwards
#define STACKSIZE (1 << 12)
static struct co coroutine[MAX_CO];
static struct co *current;
// static int co_cnt; // to record the num of coroutine
static void *tos; // top of stack


void co_init() {
    current = 0;
    return;
}

struct co* co_start(const char *name, func_t func, void *arg) {
    int pre;
    for (pre = 0; pre < MAX_CO; pre++) {
        if (coroutine[pre].state == COROUTINE_DEAD) {
            break;
        }
    }
    if (pre == MAX_CO) {
        printf("NO ENOUGH THREADS!\n");
        assert(0);
    }

    strcpy(coroutine[pre].name, name);
    // coroutine[co_cnt].name = name;
    coroutine[pre].func = func;
    coroutine[pre].state = COROUTINE_READY;

    if (tos == NULL) {
        tos = (void *)&arg;
    }
    tos += STACKDIR STACKSIZE;
    char arg_n[STACKDIR(tos - (void *)&arg)];
    coroutine[pre].coarg = arg_n;

    current = &coroutine[pre];

    if (setjmp(coroutine[pre].buf)) {
        return current;
    }
    /* else {
       func(arg);
       }
       */
    // func(arg); // Test #2 hangs
    return (struct co*)(coroutine + pre);
}

void co_yield() {
    int val = setjmp(current->buf);
    if (val == 0) {
        int go;
        for (go = 0; go < MAX_CO; go++) {
            if (coroutine[go].state != COROUTINE_DEAD
                    && coroutine[go].state != COROUTINE_RUNNING) {
                break;
            }
        }
        if (go == MAX_CO) {
            printf("NO ACCESSIBLE COROUTINE!\n");
            return;
        }
        longjmp(coroutine[go].buf, 1);
    } else {
        return;
    }
}

void co_wait(struct co *thd) {
    if (thd == NULL) {
        return;
    }

    int state = thd->state;
    switch(state) {
        case COROUTINE_READY:


        default:
            printf("Wrong State!\n");
            assert(0);
    }

    memset(thd, 0, sizeof(struct co));
    return;
}

