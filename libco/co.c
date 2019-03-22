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
    coroutine[pre].state = COROUTINE_READY;
    coroutine[pre].stack = malloc(STACKSIZE);
    coroutine[pre].stack += STACKSIZE;

    if (setjmp(coroutine[pre].buf)) {
            printf("***\n");
                asm volatile("mov " SP ", %0; mov %1, " SP :
                             "=g"(current->stack_backup) :
                             "g"(current->stack) :
                             SP_C);
        printf("9\n");
        current->state = COROUTINE_RUNNING;
        current->func(current->coarg);
        // func(arg); // Test #2 hangs
        asm volatile("mov %0," SP : : "g"(current->stack_backup) : SP_C);
    } else {
        return &coroutine[pre];
    }
    printf("33\n");
    current->state = COROUTINE_SUSPEND;
    longjmp(retbuf, 1);
}

void co_yield() {
    if (!setjmp(current->buf)) {
        //    printf("###\n");
        int go;
        for (go = 0; go < MAX_CO; go++) {
            if (coroutine[go].state != COROUTINE_DEAD
                    && coroutine[go].state != COROUTINE_RUNNING) {
                break;
            }
        }
        printf("8\n");
        if (go == MAX_CO) {
            printf("NO ACCESSIBLE COROUTINE!\n");
            return;
        }
        current->state = COROUTINE_SUSPEND;
        // printf("the upper arg: %s\n", (char *)current->coarg);
        current = &coroutine[go];
        printf("go %d\n", go);
        current->state = COROUTINE_RUNNING;
        // printf("the lower arg: %s\n", (char *)current->coarg);
        longjmp(coroutine[go].buf, 1);
    } else {
        return;
    }
}

void co_wait(struct co *thd) {
    printf("\nNOTICE!!!!\n");
    if (setjmp(retbuf)) {
        printf("\nNOTICE RET !!!\n");
        free(current->stack);
        free(current->stack_backup);
        memset(current, 0, sizeof(struct co));
        return;
    }

    if (thd == NULL) {
        printf("EMPTY THD!\n");
        return;
    }

    switch(thd->state) {
        case COROUTINE_READY:
            if (current != NULL) {
                if (current->state == COROUTINE_RUNNING) {
                    current->state = COROUTINE_SUSPEND;
                }
            }

            current = thd;
            thd->state = COROUTINE_RUNNING;
            printf("44\n");
            asm volatile("mov " SP ", %0; mov %1, " SP :
                         "=g"(thd->stack_backup) :
                         "g"(thd->stack) :
                         SP_C);
            // printf("2\n");
            thd->func(thd->coarg);
            printf("finish one!\n");
            /*
               asm volatile("mov " SP ", %0; mov %1, " SP :
               "=g"(thd->stack) :
               "g"(thd->stack_backup) :
               SP_C);
            */

            /*
            if (current->stack != NULL) {
                free(current->stack);
            }
            */
            printf("free stack\n");
            asm volatile("mov %0," SP : : "g"(current->stack_backup) : SP_C);
            printf("???\n");
            //free(current->stack_backup);
            printf("free stack_backup\n");
            memset(thd, 0, sizeof(struct co));
            return;
            break;
            /*
               case COROUTINE_SUSPEND:
               return;
               break;
               */
        default:
            printf("Wrong State!\n");
            assert(0);
    }

    free(thd->stack);
    free(thd->stack_backup);
    memset(thd, 0, sizeof(struct co));
    return;
}

