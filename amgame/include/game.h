#include <am.h>
#include <amdev.h>
#include <klib.h>

#define SIDE 24

static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
