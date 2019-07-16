#include <common.h>
#include <dir.h>
#include <fs.h>
#include <kernel.h>
#include <klib.h>
#include <vfs.h>

/*======== global variables =======*/
mptable_t mptable[MAXMOUNTPOINT];
int mptable_cnt;
/*????????????????vfs_init???? */

int find_fd(task_t *curr_task) {
  for (int i = 0; i < NOFILE; i++) {
    if (!curr_task->fildes[i]) {
      curr_task->fildes[i] = pmm->alloc(sizeof(file_t));
      return i;
    }
  }
  log("no free fd!");
  return -1;
}

/*======== vfs function =======*/

void vfs_init() {
  // TODO
  // devfs_init();
  mptable_cnt = 0;
  strcpy(cur_task->pwd, "/dev");
  vfs->mount("/dev", &devfs);
  // 没有实际挂载的设备，设为NULL
  devfs.ops->init(&devfs, "devfs", NULL);
  return;
}

int vfs_access(const char *path, int mode) {
  // TODO
  /*???????????????????0?? */
  return 0;
}

/*???mount point table? */
/*?????????????????????????????? */
int vfs_mount(const char *path, filesystem_t *fs) {
  if (mptable_cnt >= MAXMOUNTPOINT) {
    log("cannot mount more filesystem!");
    return -1;
  }

  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return -1;

  mptable[mptable_cnt].mount_point = path;
  mptable[mptable_cnt++].fs = fs;
  return 0;
}

int vfs_unmount(const char *path) {
  for (int i = 0; i < mptable_cnt; i++) {
    if (strcmp(mptable[i].mount_point, path) == 0) {
      mptable[i].mount_point = mptable[--mptable_cnt].mount_point;
      mptable[i].fs = mptable[mptable_cnt].fs;
      return 0;
    }
  }
  log("unmount failed, no such mount point");
  return -1;
}

int vfs_mkdir(const char *path) {
  // TODO
  return 0;
}

int vfs_rmdir(const char *path) {
  // TODO
  return 0;
}

int vfs_link(const char *oldpath, const char *newpath) {
  // TODO
  // ?oldpath?inode number??newpath?
  // ??newpath???????
  return 0;
}
int vfs_unlink(const char *path) {
  // TODO
  // unlink???path?????????
  // ?refcnt??????????link????inode?????
  return 0;
}

int vfs_open(const char *path, int flags) {
  // TODO
  /*???????fd?????Lookup????inode????fd?inode???????fd
   */
  int new_fd = find_fd(cur_task);
  if (new_fd == -1) {
    log("open file failed, no free fd!");
    return -1;
  }
  filesystem_t *tmp_fs = NULL;
  size_t tmp_mount_point_len = 0;
  for (int i = 0; i < mptable_cnt; i++) {
    if (tmp_mount_point_len < strlen(mptable[i].name)) {
      tmp_mount_point_len = strlen(mptable[i].name);
      if (strncmp(mptable[i].name, path, tmp_mount_point_len) == 0) {
        tmp_fs = mptable[i].fs;
      }
    }
  }
  assert(tmp_fs);
  inode_t *inode = tmp_fs->ops->lookup(tmp_fs, path, flags);
  cur_task->fildes[new_fd]->inode = inode;
  inode->ops->open(cur_task->fildes[new_fd], flags);
  return new_fd;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
  // TODO
  file_t *cur_file = cur_task->fildes[fd];
  if (cur_file) {
    return cur_file->inode->ops->read(cur_file, buf, nbyte);
  } else {
    log("wrong fd!");
    return -1;
  }
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
  // TODO
  file_t *cur_file = cur_task->fildes[fd];
  if (cur_file) {
    return cur_file->inode->ops->write(cur_file, buf, nbyte);
  } else {
    log("wrong fd!");
    return -1;
  }
}

off_t vfs_lseek(int fd, off_t offset, int whence) {
  // TODO
  return 0;
}

int vfs_close(int fd) {
  // TODO
  return 0;
}

MODULE_DEF(vfs){
    .init = vfs_init,
    .access = vfs_access,
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .mkdir = vfs_mkdir,
    .rmdir = vfs_rmdir,
    .link = vfs_link,
    .unlink = vfs_unlink,
    .open = vfs_open,
    .read = vfs_read,
    .write = vfs_write,
    .lseek = vfs_lseek,
    .close = vfs_close,
};