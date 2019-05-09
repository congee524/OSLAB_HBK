#include <common.h>
#include <klib.h>

static void kmt_init() {
    // TODO
}

static int kmt_create(task_t *task, const char *name, \
        void (*entry)(void *arg), void *arg) {
    // TODO
    return 0;
}

static void kmt_teardown(task_t *task) {
    // TODO
}

static void kmt_spin_init(spinlock_t *lk) {
    // TODO
}

static void kmt_spin_lock(spinlock_t *lk){
    // TODO
}

static void kmt_spin_unlock(spinlock_t *lk) {
    // TODO
}

static void kmt_sem_init(sem_t *sem, const char *name, int value){
    // TODO
}

static void kmt_sem_wait(sem_t *sem) {
    // TODO
}

static void kmt_sem_signal(sem_t *sem) {
    // TODO
}

MODULE_DEF(kmt) {
    .init = kmt_init,
    .create = kmt_create,
    .teardown = kmt_teardown,
    .spin_lock = kmt_spin_lock,
    .spin_unlock = kmt_spin_unlock,
    .sem_init = kmt_sem_init,
    .sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal,
};



