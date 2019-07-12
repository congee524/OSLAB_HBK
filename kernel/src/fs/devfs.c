#include <common.h>
#include <devices.h>
#include <vfs.h>

/*注意到kernel.h中有device的module devops直接拿来用 */

void devfs_init(filesystem_t *fs, const char *name, dev_t *dev) {
  // TODO
  return;
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO
  return;
}

int devfs_close(inode_t *inode) {
  // TODO
  return;
}

fsops_t devfs_ops = {
    .init = devfs_init,
    .lookup = devfs_lookup,
    .close = devfs_close,
};

filesystem_t devfs = {
    .ops = devfs_ops,
};