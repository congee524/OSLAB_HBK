#include <blkfs.h>
#include <common.h>
#include <devices.h>
#include <dir.h>
#include <vfs.h>
/*
blkfs的bitmap和目录项都存在内存中，汗。。。
额外开一个结构体
 */

/*======= blkfs_helper =======*/
bit_map bitmap[BLOCK_NUM];  // indicate the block used or not
extern inode_t *itable[];
int find_free_bit() {
  int i;
  for (i = 0; i < BLOCK_NUM; i++) {
    if (!bitmap[i].used) return i;
  }
  log("no free block!");
  return -1;
}
/*======= blkfs_fsops =======*/

void blkfs_init(filesystem_t *fs, const char *name, device_t *dev) {
  // TODO:
  fs->dev = dev;
  fs->name = name;
  return;
}

inode_t *blkfs_lookup(filesystem_t *fs, const char *path, int flags) {
  // TODO:
  int ret = path_parse(path);
  return itable[ret];
}

int blkfs_close(inode_t *inode) {
  // TODO:
  return 0;
}

fsops_t blkfs_ops = {
    .init = blkfs_init,
    .lookup = blkfs_lookup,
    .close = blkfs_close,
};

/*======= blkfs_inodeops =======*/

int blkfs_iopen(file_t *file, int flags) {
  file->refcnt++;
  file->flags = flags;
  file->offset = 0;
  return 0;
}

int blkfs_iclose(file_t *file) {
  file->refcnt--;
  file->offset = 0;
  return 0;
}

ssize_t blkfs_iread(file_t *file, char *buf, size_t size) {
  if (file->flags & O_WRONLY) {
    log("no access to read!");
    return 0;
  }
  blk_inode *b_inode = file->inode->ptr;
  device_t *dev = file->inode->fs->dev;
  if (file->offset + size > b_inode->fsize) {
    if (file->offset >= b_inode->fsize) {
      return 0;
    }
    size = b_inode->fsize - file->offset;
  }
  int block_fid = file->offset / BLOCK_SIZE;
  size_t pre_block_off = file->offset - block_fid * BLOCK_SIZE;
  char *tmp_buf = buf;
  size_t tmp_size = size;
  while (tmp_size + pre_block_off >= BLOCK_SIZE) {
    size_t nread = dev->ops->read(
        dev, (b_inode->ptr_point[block_fid] * BLOCK_SIZE) + pre_block_off,
        tmp_buf, BLOCK_SIZE - pre_block_off);
    tmp_buf += nread;
    tmp_size -= nread;
    pre_block_off = 0;
    block_fid++;
  }
  dev->ops->read(dev,
                 (b_inode->ptr_point[block_fid] * BLOCK_SIZE) + pre_block_off,
                 tmp_buf, tmp_size);
  file->offset += size;
  return size;
}

ssize_t blkfs_iwrite(file_t *file, const char *buf, size_t size) {
  if (!(file->flags & O_WRONLY) && !(file->flags & O_RDWR)) {
    log("no access to write!");
    return 0;
  }

  blk_inode *b_inode = file->inode->ptr;
  device_t *dev = file->inode->fs->dev;
  if (file->offset + size > b_inode->fsize) {
    int low_fid = b_inode->fsize / BLOCK_SIZE + 1;
    if (file->offset + size >= MAX_FSIZE) {
      b_inode->fsize = MAX_FSIZE;
    } else {
      b_inode->fsize = file->offset + size;
    }
    int up_fid = b_inode->fsize / BLOCK_SIZE;
    while (low_fid <= up_fid) {
      b_inode->ptr_point[low_fid++] = find_free_bit();
    }
    size = b_inode->fsize - file->offset;
  }
  int block_fid = file->offset / BLOCK_SIZE;
  size_t pre_block_off = file->offset - block_fid * BLOCK_SIZE;
  char *tmp_buf = pmm->alloc(size);
  strcpy(tmp_buf, buf);
  size_t tmp_size = size;
  while (tmp_size + pre_block_off >= BLOCK_SIZE) {
    size_t nwrite = dev->ops->write(
        dev, (b_inode->ptr_point[block_fid] * BLOCK_SIZE) + pre_block_off,
        tmp_buf, BLOCK_SIZE - pre_block_off);
    tmp_buf += nwrite;
    tmp_size -= nwrite;
    pre_block_off = 0;
    block_fid++;
  }
  dev->ops->write(dev,
                  (b_inode->ptr_point[block_fid] * BLOCK_SIZE) + pre_block_off,
                  tmp_buf, tmp_size);
  file->offset += size;
  return size;
}

off_t blkfs_ilseek(file_t *file, off_t offset, int whence) {
  // TODO:
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

int blkfs_imkdir(const char *name) { return -1; }

int blkfs_irmdir(const char *name) { return -1; }

int blkfs_ilink(const char *name, inode_t *inode) { return -1; }

int blkfs_iunlink(const char *name) { return -1; }

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
