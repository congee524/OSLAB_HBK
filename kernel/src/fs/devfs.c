#include <common.h>
#include <devices.h>
#include <vfs.h>

/*注意到kernel.h中有device的module devops直接拿来用 */

void devfs_init(filesystem_t *fs, const char *name, dev_t *dev) {
  // TODO
  // devfs？
  return;
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO
  // 暂时不考虑挂载的不同的文件系统，统一弄个inode出阿里
  char *resolvedpath;
  resolvedpath = realpath(path, resolvedpath);
  if (!reslovedpath) return NULL;

  inode_t *ret = path_parse(const char *reslovedpath) ； return ret;
}

int devfs_close(inode_t *inode) {
  // TODO
  return;
}

fsops_t devfs_ops = {
    .init = &devfs_init,
    .lookup = &devfs_lookup,
    .close = &devfs_close,
};

filesystem_t devfs = {
    .ops = devfs_ops,
};