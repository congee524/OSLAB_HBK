#include <common.h>
#include <dir.h>
#include <kernel.h>
#include <klib.h>
/*use strtok */

static char pwd[MAXPATHLEN];
// TODO 记得初始化
// dir_t *rootdirt;
inode_t *rootdir_inode;

/*转换为绝对路径，还要进行路径解析 */
char *realpath(const char *path, char *resolvedpath) {
  if (!path) {
    log("no path!");
    return NULL;
  }

  if (!resolvedpath) {
    log("resolved path pointer is NULL!");
    return NULL;
  }

  if (path[0] == '/') {
    strcpy(resolvedpath, path);
    return resolvedpath;
  } else {
    strcpy(resolvedpath, pwd);
    /*多个‘/’视作一个 */
    strcat(resolvedpath, "/");
    strcat(resolvedpath, path);
    return resolvedpath;
  }
}

inode_t *path_parse(const char *path) {
  /*
  // 解析中调用绝对路径转换？看实现
  char *resolvedpath;
  resolvedpath = realpath(path, resolvedpath);
  if (!reslovedpath) return NULL;
   */
  /*
  想到一个严重的问题，应当根据挂载点选择根目录才对
  后面修改
   */
  if (!path) {
    log("no path!");
    return NULL;
  }
  if (path[0] != '/') {
    log("not resolvedpath!");
    return NULL;
  }

  dir_t *predir;
  inode_t *ret = rootdir_inode;
  char *pch = strtok(path, "/");
  while (pch != NULL && ret->type == VFILE_DIR) {
    predir = (dir_t *)(ret->ptr);
    if (strcmp(pch, "..") == 0) {
      ret = predir->pa;
    } else if (strcmp(pch, ".") == 0) {
      ret = predir->self;
    } else {
      int i;
      for (i = 0; i < MAXDIRITEM; i++) {
        if (predir->names[i] && strcmp(pch, predir->names[i]) == 0) {
          ret = predir->inodes[i];
          break;
        }
      }
      if (i >= MAXDIRITEM) {
        log("cannot find %s in %s!", pch, path);
        return NULL;
      }
    }
    pch = strtok(NULL, "/");
  }
  if (pch) {
    log("%s is not a sub_direct_item in %s", pch, path);
    return NULL;
  }
  return ret;
}