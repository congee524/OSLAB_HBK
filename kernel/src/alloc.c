#include <alloc.h>
#include <common.h>
#include <klib.h>

static uintptr_t pm_start, pm_end;

static void pmm_init() {
  // need to adjust the code according to the _heap.start and end!!!!
  pm_start = (uintptr_t)_heap.start;
  pm_end = (uintptr_t)_heap.end;
}

static t_block extend_heap(t_block last, size_t size) {
  t_block pre;
  pre = sbrk(0);
  if (sbrk(BLOCK_SIZE + size) == (void *)-1) {
    return NULL;
  } else {
    pre->size = size;
    pre->next = NULL;
    pre->free = 0;
    if (last) {
      last->next = pre;
    }
    return pre;
  }
}

static t_block find_block(t_block *last, size_t size) {
  t_block pre = base;
  if (pre && !(pre->free && b->size >= size)) {
    // "*last" point the address of the pointer "last"
    *last = pre;
    pre = pre->next;
  }
  return pre;
}

static void split_block(t_block pre, size_t size) {
  t_block new;
  new = &pre->data + size;
  new->size = b->size - size - BLOCK_SIZE;
  new->free = 1;
  new->next = pre->next;
  pre->next = new;
  pre->size = size;
}

static void *kalloc(size_t size) {
  spin_lock(&alloc_lk);
  // TODO()

  t_block pre, last;
  pre = last = NULL;
  size = align4(size);
  if (base == NULL) {
    pre = extend_heap(last, size);
    if (pre == NULL) {
      return NULL;
    } else {
      base = pre;
    }
  } else {
    last = base;
    pre = find_block(&last, size);
    if (pre) {
      if (pre->size - size > (BLOCK_SIZE + 8)) {
        split_block(pre, size);
      }
      pre->free = 0;
    } else {
      pre = extend_heap(last, size);
      if (pre == NULL) {
        return NULL;
      }
    }
  }

  spin_unlock(&alloc_lk);
  return (void *)&pre->data;
}

static void kfree(void *ptr) {
  spin_lock(&alloc_lk);
  // TODO()
  spin_unlock(&alloc_lk);
}

MODULE_DEF(pmm){
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
