#ifndef __DIR_H__
#define __DIR_H__
/*需要一个dir相关的函数，处理各种路径，主要是把相对路径转化为绝对路径，以及根据绝对路径得到文件的inode
 */

typedef struct DIRE dir_t;

char *realpath(const char *path, char *resolvedpath);
int path_parse(const char *path);
int find_parent_dir(const char *path, char *fname);
int dir_last_item(char *path, char *name);

struct DIRE {
  int self;
  int pa;
  char *names[MAXDIRITEM]; /*不要初始化, 用的时候再分配 */
  int inodes_ind[MAXDIRITEM];
};

// char pwd[MAXPATHLEN];
// 所有线程都应该有一个pwd，放到task_t结构体中

#endif