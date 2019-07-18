#include <common.h>
#include <devices.h>
#include <dir.h>
#include <vfs.h>

extern inode_t *itable[];

/*======= procfs_fsops =======*/

void procfs_init(filesystem_t *fs, const char *name, device_t *dev) {
  // TODO:
  fs->dev = dev;
  fs->name = name;
  return;
}

inode_t *procfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO:
  int ret = path_parse(path);
  return itable[ret];
}

int procfs_close(inode_t *inode) {
  // TODO:
  return 0;
}

fsops_t procfs_ops = {
    .init = procfs_init,
    .lookup = procfs_lookup,
    .close = procfs_close,
};

/*======= procfs_inodeops =======*/

inodeops_t procfs_iops;

int procfs_iopen(file_t *file, int flags) {
  file->refcnt++;
  file->flags = flags;
  file->offset = 0;
  return 0;
}

int procfs_iclose(file_t *file) {
  file->refcnt--;
  file->offset = 0;
  return 0;
}

ssize_t procfs_iread(file_t *file, char *buf, size_t size) {  // TODO:
  return 0;
}

ssize_t procfs_iwrite(file_t *file, const char *buf, size_t size) { return -1; }

off_t procfs_ilseek(file_t *file, off_t offset, int whence) {
  // TODO:
  switch (whence) {
    case SEEK_SET:
      file->offset = offset;
      break;
    case SEEK_CUR:
      file->offset += offset;
      break;
    case SEEK_END:
      file->offset = ((blk_inode *)(file->inode->ptr))->fsize;
      break;
    default:
      return -1;
  }
  return file->offset;
}

int procfs_imkdir(const char *path) { return -1; }

int procfs_irmdir(const char *path) { return -1; }

int procfs_itouch(const char *path) { return -1; }

int procfs_irm(const char *path) { return -1; }

int procfs_ilink(const char *name, inode_t *inode) { return -1; }

int procfs_iunlink(const char *name) { return -1; }

inodeops_t procfs_iops = {
    .open = procfs_iopen,
    .close = procfs_iclose,
    .read = procfs_iread,
    .write = procfs_iwrite,
    .lseek = procfs_ilseek,
    .mkdir = procfs_imkdir,
    .rmdir = procfs_irmdir,
    .touch = procfs_itouch,
    .rm = procfs_irm,
    .link = procfs_ilink,
    .unlink = procfs_iunlink,
};

filesystem_t procfs = {
    .ops = &procfs_ops,
    .iops = &procfs_iops,
};
