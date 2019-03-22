#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include "co.h"

#define COROUTINE_DEAD 0
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3
#define MAX_CO 10
#define STACKSIZE (1 << 12)

#if defined(__i386__)
    #define SP "%%esp"
#elif defined(__x86_64__)
    #define SP "%%rsp"
#endif

#if defined(__i386__)
#define SP_C "%esp"
#elif defined(__x86_64__)
#define SP_C "%rsp"
#endif

struct co {
    char name[128];
    func_t func;
    jmp_buf buf; // to save the state of present function
    int state; // record the state of routine
    void *coarg;
    void *stack;
    void *stack_backup;
};

// #define STACKDIR - // set - for downwards
static struct co coroutine[MAX_CO];
static struct co *current;
// static int co_cnt; // to record the num of coroutine
// static void *tos; // top of stack
static jmp_buf retbuf;

void co_init() {
    current = NULL;
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
    coroutine[pre].coarg = arg;
    coroutine[pre].state = COROUTINE_RUNNING;
    coroutine[pre].stack = malloc(STACKSIZE);
    coroutine[pre].stack += STACKSIZE;

    if (setjmp(coroutine[pre].buf)) {
            //printf("***\n");
                asm volatile("mov " SP ", %0; mov %1, " SP :
                             "=g"(current->stack_backup) :
                             "g"(current->stack) :
                             SP_C);
        //printf("9\n");
        current->func(current->coarg);
        // func(arg); // Test #2 hangs
        asm volatile("mov %0," SP : : "g"(current->stack_backup) : SP_C);
    } else {
        return &coroutine[pre];
    }
    //printf("33\n");
    current->state = COROUTINE_SUSPEND;
    longjmp(retbuf, 1);
}

int go=0;
void co_yield() {
    if (!setjmp(current->buf)) {
        current->state = COROUTINE_SUSPEND;
        //    printf("###\n");
        while(1) {
            go++;
            if(go==MAX_CO)go=0;
            if (coroutine[go].state != COROUTINE_DEAD){
                   // && coroutine[go].state != COROUTINE_RUNNING) {
                break;
            }
        }
        //printf("8\n");
        if (go == MAX_CO) {
            printf("NO ACCESSIBLE COROUTINE!\n");
            return;
        }
        // printf("the upper arg: %s\n", (char *)current->coarg);
        current = &coroutine[go];
        //printf("go %d\n", go);
        current->state = COROUTINE_RUNNING;
        // printf("the lower arg: %s\n", (char *)current->coarg);
        longjmp(coroutine[go].buf, 1);
    } else {
        return;
    }
}

void co_wait(struct co *thd) {
    //printf("\nNOTICE! %s\n", thd->name);
    //printf("STATE %d\n", thd->state);
    if (setjmp(retbuf)) {
        //printf("\nNOTICE RET !!!\n");
        //free(current->stack);
        //free(current->stack_backup);
        memset(current, 0, sizeof(struct co));
        return;
    }

    if (thd == NULL) {
        printf("EMPTY THD!\n");
        return;
    }

    printf("STATE %d\n", thd->state);
    switch(thd->state) {
        case COROUTINE_SUSPEND:
            break;
        case COROUTINE_RUNNING:
            current = thd;
            longjmp(current->buf, 1);
            break;
        default:
            //printf("COROUTINE_RUNNING\n");
            printf("Wrong State %d!\n", thd->state);
            assert(0);
    }

    // free(thd->stack);
    // free(thd->stack_backup);
    memset(current, 0, sizeof(struct co));
    assert(current->state==COROUTINE_DEAD);
    return;
}

