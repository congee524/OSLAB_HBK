typedef unsigned int uint;

// Record the current call stack in pcs[] by following the %ebp chain.
void getcallerpcs(void *v, uint pcs[]) {
  uint *ebp;
  int i;

  ebp = (uint *)v - 2;
  for (i = 0; i < 10; i++) {
    if (ebp == 0 || ebp < (uint *)KERNBASE || ebp == (uint *)0xffffffff) break;
    pcs[i] = ebp[1];       // saved %eip
    ebp = (uint *)ebp[0];  // saved %ebp
  }
  for (; i < 10; i++) pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int holding(struct spinlock *lock) { return lock->locked && lock->cpu == cpu; }

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off
void pushcli(void) {
  int eflags;

  eflags = readeflags();
  cli();
  if (cpu->ncli++ == 0) cpu->intena = eflags & FL_IF;
}

void popcli(void) {
  if (readeflags() & FL_IF) panic("popcli - interruptible");
  if (--cpu->ncli < 0) panic("popcli");
  if (cpu->ncli == 0 && cpu->intena) sti();
}