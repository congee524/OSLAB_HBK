#include <common.h>
#include <dir.h>
#include <kernel.h>
#include <klib.h>
/*use strtok */

static char pwd[MAXPATHLEN];

/*转换为绝对路径，还要进行路径解析 */
char *realpath(const char *path, char *resolvedpath) {
  if (!path) {
    log("no path!");
    return NULL;
  }

  if (!resolvedpath) resolvedpath = pmm->alloc(MAXPATHLEN);

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