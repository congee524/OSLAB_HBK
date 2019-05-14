#include <kernel.h>

#define BLOCK_SIZE 12
// align the size by 4 byte
#define align4(x) (((((x)-1) >> 2) << 2) + 4)

void *base = NULL;

typedef struct s_block *t_block;
struct s_block {
  size_t size;
  int free;
  t_block next;
  char data;  // denote the first byte of the memory block
};
