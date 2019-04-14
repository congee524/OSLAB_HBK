#include <alloc.h>
#include <common.h>
#include <klib.h>

static uintptr_t pm_start, pm_end, pm_pre;

static void pmm_init() {
  // need to adjust the code according to the _heap.start and end!!!!
  pm_start = (uintptr_t)_heap.start;
  pm_end = (uintptr_t)_heap.end;
  pm_pre = pm_start;
}

static void *sbrk(intptr_t increment) {
  pm_pre += increment;
  if (pm_pre > pm_end || pm_pre < pm_start) {
    pm_pre -= increment;
    return (void *)-1;
  }
  return (void *)pm_pre;
}

static t_block extend_heap(t_block last, size_t size) {
  t_block pre;
  pre = sbrk(0);
  printf("extend %p\n", pre);
  if (sbrk(BLOCK_SIZE + size) == (void *)-1) {
    return NULL;
  } else {
    pre->size = size;
    pre->next = NULL;
    pre->free = 0;
    if (last) {
      last->next = pre;
    }
    printf("test free %d\n", pre->free);
    return pre;
  }
}

static t_block find_block(t_block *last, size_t size) {
  t_block pre = base;
  if (pre != NULL && !((pre->free == 1) && (pre->size) >= size)) {
    // "*last" point the address of the pointer "last"
    printf("!!!!!!!\n");
    *last = pre;
    pre = pre->next;
    printf("pre to next at %p\n", pre);
    printf("pre->free %d\n", pre->free);
  }
  return pre;
}

static void split_block(t_block pre, size_t size) {
  t_block new;
  new = (t_block)(&pre->data + size);
  new->size = pre->size - size - BLOCK_SIZE;
  new->free = 1;
  new->next = pre->next;
  pre->next = new;
  pre->size = size;
}

static void merge_block(t_block pre) {
  pre->size += (pre->next->size + BLOCK_SIZE);
  pre->next->free = 0;
  pre->next = pre->next->next;
}

static void *kalloc(size_t size) {
  spin_lock(&alloc_lk);
  // TODO()

  t_block pre, last;
  pre = last = NULL;
  size = align4(size);
  if (base == NULL) {
    pre = extend_heap(last, size);
    printf("base !\n");
    if (pre == NULL) {
      printf("base Need more memory!\n");
      spin_unlock(&alloc_lk);
      return NULL;
    } else {
      base = pre;
    }
  } else {
    last = base;
    pre = find_block(&last, size);
    if (pre) {
      printf("FIND! %p\n", pre);
      if (pre->size - size > (BLOCK_SIZE + 8)) {
        split_block(pre, size);
      }
      pre->free = 0;
    } else {
      pre = extend_heap(last, size);
      if (pre == NULL) {
        printf("Need more memory!\n");
        spin_unlock(&alloc_lk);
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
  if (base == NULL) {
    printf("wrong memory space!\n");
    spin_unlock(&alloc_lk);
    assert(0);
    return;
  }
  printf("ptr at %p\n", ptr);
  t_block last = base, pre = base;
  while (pre && ((void *)&pre->data != ptr)) {
    last = pre;
    printf("pre now at %p\n", &pre->data);
    pre = pre->next;
  }
  if (pre == NULL) {
    printf("No such memory space!\n");
    spin_unlock(&alloc_lk);
    assert(0);
    return;
  }
  pre->free = 1;
  if (pre != base && last->free == 1) {
    merge_block(last);
  }
  if (pre->next != NULL && pre->next->free == 1) {
    merge_block(pre);
  }
  spin_unlock(&alloc_lk);
  return;
}

MODULE_DEF(pmm){
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
