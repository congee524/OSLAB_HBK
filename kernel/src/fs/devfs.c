#include <common.h>
#include <devices.h>
#include <dir.h>
#include <fs.h>
#include <klib.h>
#include <vfs.h>

/*======= devfs_inodeops =======*/
int devfs_iopen(file_t *file, int flags) {
  file->offset = 0;
  file->refcnt++;
  return 0;
}

int devfs_iclose(file_t *file) {
  file->offset = 0;
  file->refcnt--;
  return 0;
}

ssize_t devfs_iread(file_t *file, char *buf, size_t size) {
  device_t *dev = (device_t *)(file->inode->ptr);
  ssize_t nread = dev->ops->read(dev, file->offset, buf, size);
  file->offset += nread;
  return nread;
}

ssize_t devfs_iwrite(file_t *file, const char *buf, size_t size) {
  device_t *dev = (device_t *)(file->inode->ptr);
  ssize_t nwrite = dev->ops->write(dev, file->offset, buf, size);
  file->offset += nwrite;
  return nwrite;
}

off_t devfs_ilseek(file_t *file, off_t offset, int whence) {
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

int devfs_imkdir(const char *name) { return -1; }

int devfs_irmdir(const char *name) { return -1; }

int devfs_ilink(const char *name, inode_t *inode) { return -1; }

int devfs_iunlink(const char *name) { return -1; }

inodeops_t devfs_iops = {
    .open = devfs_iopen,
    .close = devfs_iclose,
    .read = devfs_iread,
    .write = devfs_iwrite,
    .lseek = devfs_ilseek,
    .mkdir = devfs_imkdir,
    .rmdir = devfs_irmdir,
    .link = devfs_ilink,
    .unlink = devfs_iunlink,
};

/*======= devfs_fsops =======*/
// from dev.c
extern device_t *devices[];
extern int dev_cnt;

void devfs_init(filesystem_t *fs, const char *name, device_t *dev) {
  // TODO
  // ????
  // dev->ops->init();
  fs->name = name;
  fs->dev = dev;

  // devfs直接挂载所有的设备，分配inode
  // set the root dir of devfs
  fs->itable[0] = pmm->alloc(sizeof(struct inode));
  inode_t *inode = fs->itable[0];
  inode->refcnt = 0;
  inode->ptr = pmm->alloc(sizeof(struct DIRE));
  inode->fs = fs;
  inode->ops = NULL;
  inode->type = VFILE_DIR;
  inode->fsize = sizeof(struct DIRE);
  dir_t *dev_root_dir = inode->ptr;
  dev_root_dir->self = 0;
  dev_root_dir->pa = 0;
  for (int i = 0; i < dev_cnt; i++) {
    fs->itable[i + 1] = pmm->alloc(sizeof(struct inode));
    inode_t *inode = fs->itable[i + 1];
    inode->refcnt = 0;
    inode->ptr = devices[i];
    inode->fs = fs;
    inode->ops = &devfs_iops;
    inode->type = VFILE_FILE;
    inode->fsize = sizeof(struct divice);
    dev_root_dir->names[i] = pmm->alloc(MAXNAMELEN);
    strcpy(dev_root_dir->names[i], devices[i]->name);
    dev_root_dir->inodes_ind[i] = i + 1;
  }
  return;
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO
  // 暂时不考虑挂载的不同的文件系统，统一弄个inode出来
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return NULL;
  inode_t *ret = path_parse(resolvedpath);
  return ret;
}

int devfs_close(inode_t *inode) {
  // TODO
  return 0;
}

fsops_t devfs_ops = {
    .init = devfs_init,
    .lookup = devfs_lookup,
    .close = devfs_close,
};

filesystem_t devfs = {
    .ops = &devfs_ops,
    .dev = NULL,
};