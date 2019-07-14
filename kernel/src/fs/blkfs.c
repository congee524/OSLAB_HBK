#include <common.h>
#include <dir.h>
#include <vfs.h>

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev) {
  // TODO
  return;
}

inode_t *blkfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO
  // 暂时不考虑挂载的不同的文件系统，统一弄个inode
}

int blkfs_close(inode_t *inode) {
  // TODO
  return 0;
}

fsops_t blkfs_ops = {
    .init = blkfs_init,
    .lookup = blkfs_lookup,
    .close = blkfs_close,
};

/*======= blkfs_inodeops =======*/
int blkfs_iopen(file_t *file, int flags) { return 0; }

int blkfs_iclose(file_t *file) { return 0; }

ssize_t blkfs_iread(file_t *file, char *buf, size_t size) { return nread; }

ssize_t blkfs_iwrite(file_t *file, const char *buf, size_t size) {
  return nwrite;
}

off_t blkfs_ilseek(file_t *file, off_t offset, int whence) {}

int blkfs_imkdir(const char *name) {}

int blkfs_irmdir(const char *name) {}

int blkfs_ilink(const char *name, inode_t *inode) {}

int blkfs_iunlink(const char *name) {}

inodeops_t blkfs_iops = {
    .open = blkfs_iopen,
    .close = blkfs_iclose,
    .read = blkfs_iread,
    .write = blkfs_iwrite,
    .lseek = blkfs_ilseek,
    .mkdir = blkfs_imkdir,
    .rmdir = blkfs_irmdir,
    .link = blkfs_ilink,
    .unlink = blkfs_iunlink,
};

filesystem_t blkfs = {
    .ops = &blkfs_ops,
    .blk = NULL,
};