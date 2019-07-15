/*
#include <common.h>
#include <devices.h>
#include <dir.h>
#include <vfs.h>

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev) {
  // TODO
  return;
}

inode_t *blkfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO
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
*/
/*======= blkfs_inodeops =======*/
/*
int blkfs_iopen(file_t *file, int flags) { return 0; }

int blkfs_iclose(file_t *file) { return 0; }

ssize_t blkfs_iread(file_t *file, char *buf, size_t size) {}

ssize_t blkfs_iwrite(file_t *file, const char *buf, size_t size) {}

off_t blkfs_ilseek(file_t *file, off_t offset, int whence) {
  switch (whence) {
    case SEEK_SET:
      file->offset = offset;
      break;
    case SEEK_CUR:
      file->offset += offset;
      break;
    case SEEK_END:
      file->offset = file->inode->fsize;
      break;
    default:
      return -1;
  }
  return file->offset;
}

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

filesystem_t blkfs[2] = {
    {
        .ops = &blkfs_ops,
        // dev_lookup are not constant, must init in function
        //.dev = dev_lookup("ramdisk0"),
    },
    {
        .ops = &blkfs_ops,
        //.dev = dev_lookup("ramdisk1"),
    },
};
*/