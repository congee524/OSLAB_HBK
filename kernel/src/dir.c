#include <common.h>
#include <dir.h>
#include <kernel.h>
#include <klib.h>
/*use strtok */

// TODO 记得初始化
// dir_t *rootdirt;
extern int mptable_cnt;
extern mptable_t mptable[];

/*转换为绝对路径，还要进行路径解析 */
char *realpath(const char *path, char *resolvedpath) {
  kmt->spin_lock(&print_lk);
  if (!path) {
    log("no path!");
    kmt->spin_unlock(&print_lk);
    return NULL;
  }

  if (!resolvedpath) {
    log("resolved path pointer is NULL!");
    kmt->spin_unlock(&print_lk);
    return NULL;
  }

  if (path[0] == '/') {
    strcpy(resolvedpath, path);
    kmt->spin_unlock(&print_lk);
    return resolvedpath;
  } else {
    printf("dir bef cur_task->pwd: %s", cur_task->pwd);
    strcpy(resolvedpath, cur_task->pwd);
    printf("dir aft cur_task->pwd: %s", cur_task->pwd);
    /*多个‘/’视作一个 */
    strcat(resolvedpath, "/");
    strcat(resolvedpath, path);
    kmt->spin_unlock(&print_lk);
    return resolvedpath;
  }
  kmt->spin_unlock(&print_lk);
}

int path_parse(filesystem_t *fs, const char *path) {
  // 解析中调用绝对路径转换？看实现
  if (!path) {
    log("no path!");
    return -1;
  }
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return -1;
  /*
  想到一个严重的问题，应当根据挂载点选择根目录才对
  后面修改
   */
  char *root_path = pmm->alloc(MAXPATHLEN);
  for (int i = 0; i < mptable_cnt; i++) {
    if (mptable[i].fs == fs) {
      strcpy(root_path, mptable[i].mount_point);
      break;
    }
  }
  char *tar_path = resolvedpath;
  while (*root_path == *tar_path) {
    root_path++;
    tar_path++;
  }
  char *tmp_path = pmm->alloc(MAXPATHLEN);
  strcpy(tmp_path, tar_path);
  // printf("first tmp_path: %s\n", tmp_path);
  dir_t *predir;
  int ret = 0;
  char *pch = strtok(tmp_path, "/");

  while (pch != NULL && fs->itable[ret]->type == VFILE_DIR) {
    predir = (dir_t *)(fs->itable[ret]->ptr);
    if (strcmp(pch, "..") == 0) {
      ret = predir->pa;
    } else if (strcmp(pch, ".") == 0) {
      ret = predir->self;
    } else {
      int i;
      for (i = 0; i < MAXDIRITEM; i++) {
        // printf("predir->name: %s\n", predir->names[i]);
        if (predir->names[i] && strcmp(pch, predir->names[i]) == 0) {
          ret = predir->inodes_ind[i];
          break;
        }
      }
      if (i >= MAXDIRITEM) {
        log("cannot find %s in %s!", pch, path);
        return -1;
      }
    }
    // printf("bef pch: %s\n", pch);
    pch = strtok(NULL, "/");
    // printf("aft pch: %s\n", pch);
  }
  if (pch) {
    log("%s is not a sub_direct_item in %s", pch, path);
    return -1;
  }
  // printf("ret: %d\n", ret);
  return ret;
}