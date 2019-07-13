#ifndef __DIR_H__
#define __DIR_H__
/*需要一个dir相关的函数，处理各种路径，主要是把相对路径转化为绝对路径，以及根据绝对路径得到文件的inode
 */

#define MAXPATHLEN 512
#define MAXDIRITEM 32

typedef struct DIRE dir_t;

char *realpath(const char *path, char *resolvedpath);

struct DIRE {
  dir_t *self;
  dir_t *pa;
  char *names[MAXDIRITEM];
  inode_t *inodes[MAXDIRITEM];
};

#endif