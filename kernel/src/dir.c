#include <common.h>
#include <dir.h>
#include <kernel.h>
#include <klib.h>
/*use strtok */

// TODO: 记得初始化
// dir_t *rootdirt;

extern inode_t *itable[];

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
  char tmp_path[MAXPATHLEN];
  if (path[0] == '/') {
    strcpy(tmp_path, path);
  } else {
    strcpy(tmp_path, cur_pwd);
    /*多个‘/’视作一个 */
    strcat(tmp_path, "/");
    strcat(tmp_path, path);
  }

  char *ptr = resolvedpath;
  char *tmp = tmp_path;
  while (*tmp != '\0') {
    switch (*tmp) {
      case '/': {
        *ptr = *tmp;
        ptr++;
        *ptr = '\0';
        while (*tmp == '/') tmp++;
        break;
      }
      case '.': {
        if (*(tmp + 1) == '.' && (*(tmp + 2) == '/' || *(tmp + 2) == '\0')) {
          *(--ptr) = '\0';
          while (*(ptr) != '/') {
            *ptr = '\0';
            ptr--;
          }
          // *ptr = '\0';
          tmp += 2;

        } else if (*(tmp + 1) == '/' || *(tmp + 1) == '\0') {
          *ptr = '\0';
          ptr--;
          //*(--ptr) = '\0';
          tmp++;
        } else {
          printf("then!\n");
          while (*tmp != '\0' && *tmp != '/') {
            *ptr = *tmp;
            ptr++;
            tmp++;
          }
          *ptr = '\0';
        }
        break;
      }
      default: {
        while (*tmp != '\0' && *tmp != '/') {
          *ptr = *tmp;
          ptr++;
          tmp++;
        }
        *ptr = '\0';
        break;
      }
    }
  }
  int len = strlen(resolvedpath);
  if (len > 1 && resolvedpath[len - 1] == '/') resolvedpath[len - 1] = '\0';
  return resolvedpath;
}

int path_parse(const char *path) {
  // 解析中调用绝对路径转换？看实现
  if (!path) {
    log("no path!");
    return -1;
  }
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return -1;
  // printf("first tmp_path: %s\n", tmp_path);
  dir_t *predir;
  int ret = 1;  // 根目录从1开始
  char *pch = strtok(resolvedpath, "/");
  if (pch == NULL) pch = "/";
  while (pch && itable[ret] && itable[ret]->type == VFILE_DIR) {
    predir = (dir_t *)(itable[ret]->ptr);
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
        // printf("ret: %d\n", ret);
        // log("cannot find %s in %s!", pch, path);
        return ret;
      }
    }
    // printf("bef pch: %s\n", pch);
    pch = strtok(NULL, "/");
    // printf("aft pch: %s\n", pch);
    while (pch && *pch == '/') pch++;
  }
  if (pch) {
    log("%s is not a sub_direct_item in %s", pch, path);
    return -1;
  }
  // printf("ret: %d\n", ret);
  pmm->free(resolvedpath);
  return ret;
}

int find_parent_dir(const char *path, char *fname) {
  if (!path) {
    log("no path!");
    return -1;
  }
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return -1;
  // printf("first tmp_path: %s\n", tmp_path);
  dir_t *predir;
  int oldret = 1, ret = 1;  // 根目录从1开始
  char *pch = strtok(resolvedpath, "/");
  while (pch && *pch == '/') pch++;
  while (pch != NULL && itable[ret] && itable[ret]->type == VFILE_DIR) {
    oldret = ret;
    strcpy(fname, pch);
    predir = (dir_t *)(itable[ret]->ptr);
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
        break;
      }
    }
    // printf("bef pch: %s\n", pch);

    pch = strtok(NULL, "/");
    // printf("aft pch: %s\n", pch);
    while (pch && *pch == '/') pch++;
  }
  pmm->free(resolvedpath);
  return oldret;
}

int dir_last_item(char *path, char *name) {
  char tmp_path[MAXPATHLEN];
  realpath(path, tmp_path);
  char *pch = strtok(tmp_path, "/");
  if (*pch != '/')
    strcpy(name, pch);
  else
    return -1;
  while ((pch = strtok(NULL, "/")) != NULL)
    if (*pch != '/') strcpy(name, pch);
  return 0;
}