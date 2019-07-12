#include <common.h>
#include <kernel.h>
#include <klib.h>

int find_fd(task_t *cur_task) {
  for (int i = 0; i < NOFILE; i++) {
    if (!cur_task->fildes[i]) {
      cur_task->fildes = pmm->alloc(sizeof(file_t));
      return i;
    }
  }
  log("no free fd!");
  return -1;
}

/*======== vfs function ======*/

void vfs_init() {
  // TODO
  devfs_init();

  return;
}

int vfs_access(const char *path, int mode) {
  // TODO
  return 0;
}

int vfs_mount(const char *path, filesystem_t *fs) {
  // TODO
  return 0;
}

int vfs_unmount(const char *path) {
  // TODO
  return 0;
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
  return 0;
}
int vfs_unlink(const char *path) {
  // TODO
  return 0;
}

int vfs_open(const char *path, int flags) {
  // TODO
  int new_fd = find_fd(current_task);
  if (new_fd == -1) {
    log("open file failed, no free fd!");
    return -1;
  }

  if (strncmp(path, "/dev/", 5) == 0) {
    // 文件系统 devfs
  }

  TODO();
  return 0;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
  // TODO
  return 0;
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
  // TODO
  return 0;
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