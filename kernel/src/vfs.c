#include <common.h>
#include <devices.h>
#include <dir.h>
#include <kernel.h>
#include <klib.h>
#include <vfs.h>

/*======== global variables =======*/
inode_t *itable[MAXINODENUM];
mptable_t mptable[MAXMOUNTPOINT];
int mptable_cnt;

extern filesystem_t devfs;
extern filesystem_t blkfs[2];
extern filesystem_t procfs;
/*======= vfs helper ========*/

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

int find_inode_ind() {
  for (int i = 1; i < MAXINODENUM; i++) {
    if (!itable[i]) return i;
  }
  log("no free inode!");
  return -1;
}

filesystem_t *find_mount_point_fs(const char *path) {
  // 只接受绝对路径
  filesystem_t *tmp_fs = NULL;
  size_t pre_mp_len = 0;
  size_t tmp_mp_len = 0;
  // printf("mptable_cnt: %d\n", mptable_cnt);
  for (int i = 0; i < mptable_cnt; i++) {
    // printf("%s\n", mptable[i].mount_point);
    tmp_mp_len = strlen(mptable[i].mount_point);
    if (pre_mp_len < tmp_mp_len && strlen(path) >= tmp_mp_len) {
      if (strncmp(mptable[i].mount_point, path, tmp_mp_len) == 0) {
        pre_mp_len = strlen(mptable[i].mount_point);
        tmp_fs = mptable[i].fs;
      }
    }
  }
  return tmp_fs;
}

/*======== vfs function =======*/

void vfs_init() {
  // TODO:
  // devfs_init();
  mptable_cnt = 0;
  for (int i = 0; i < MAX_CPU; i++) {
    pwds[i] = pmm->alloc(MAXPATHLEN);
    strncpy(pwds[i], "/", 1);
  }

  vfs->mount("/", &blkfs[0]);
  vfs->mount("/mnt", &blkfs[1]);
  vfs->mount("/proc", &procfs);
  vfs->mount("/dev", &devfs);

  // procfs 和 devfs没有实际挂载的设备，dev设为NULL
  blkfs[0].ops->init(&blkfs[0], "blkfs[0]", dev_lookup("ramdisk0"));
  blkfs[1].ops->init(&blkfs[1], "blkfs[1]", dev_lookup("ramdisk1"));
  procfs.ops->init(&procfs, "procfs", NULL);
  devfs.ops->init(&devfs, "devfs", NULL);

  return;
}

int vfs_access(const char *path, int mode) {
  // TODO:
  return 0;
}

int vfs_mount(const char *path, filesystem_t *fs) {
  if (mptable_cnt >= MAXMOUNTPOINT) {
    log("cannot mount more filesystem!");
    return -1;
  }

  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return -1;

  mptable[mptable_cnt].mount_point = pmm->alloc(MAXPATHLEN);
  strcpy(mptable[mptable_cnt].mount_point, resolvedpath);
  mptable[mptable_cnt++].fs = fs;

  if (!itable[1]) {
    assert(!strcmp(resolvedpath, "/"));
    itable[1] = pmm->alloc(sizeof(inode_t));
    inode_t *inode = itable[1];
    inode->fs = fs;
    inode->fsize = sizeof(dir_t);
    inode->ops = NULL;
    inode->ptr = pmm->alloc(sizeof(dir_t));
    dir_t *tmp_dir = inode->ptr;
    inode->refcnt = 0;
    inode->type = VFILE_DIR;
    tmp_dir->pa = 1;
    tmp_dir->self = 1;
    return 0;
  } else {
    vfs->mkdir(resolvedpath);
  }
  pmm->free(resolvedpath);
  return 0;
}

int vfs_unmount(const char *path) {
  for (int i = 0; i < mptable_cnt; i++) {
    if (strcmp(mptable[i].mount_point, path) == 0) {
      pmm->free(mptable[i].mount_point);
      mptable[i].mount_point = mptable[--mptable_cnt].mount_point;
      mptable[i].fs = mptable[mptable_cnt].fs;
      mptable[mptable_cnt].mount_point = NULL;
      mptable[mptable_cnt].fs = NULL;
      return 0;
    }
  }
  log("unmount failed, no such mount point");
  return -1;
}

int vfs_mkdir(const char *path) {
  // TODO:
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  filesystem_t *fs = find_mount_point_fs(resolvedpath);
  if (!fs) return -1;
  char name[MAXNAMELEN];
  char fname[MAXNAMELEN];
  assert(dir_last_item(resolvedpath, name) == 0);

  int pa_dir_ind = find_parent_dir(resolvedpath, fname);
  if (strcmp(fname, name) != 0) {
    log("wrong path!\n");
    pmm->free(resolvedpath);
    return -1;
  }
  assert(itable[pa_dir_ind]->type == VFILE_DIR);

  dir_t *pa_dir = itable[pa_dir_ind]->ptr;
  int inode_ind = find_inode_ind();
  int pa_dir_item_ind = 0;
  for (pa_dir_item_ind = 0; pa_dir_item_ind < MAXDIRITEM; pa_dir_item_ind++) {
    if (!pa_dir->names[pa_dir_item_ind]) break;
  }
  if (pa_dir_item_ind >= MAXDIRITEM) {
    log("parent dir has no free dir item!\n");
    pmm->free(resolvedpath);
    return -1;
  }

  pa_dir->names[pa_dir_item_ind] = pmm->alloc(MAXNAMELEN);
  strcpy(pa_dir->names[pa_dir_item_ind], fname);
  pa_dir->inodes_ind[pa_dir_item_ind] = inode_ind;
  itable[inode_ind] = pmm->alloc(sizeof(inode_t));
  inode_t *inode = itable[inode_ind];
  inode->fs = fs;
  inode->fsize = sizeof(dir_t);
  inode->ops = NULL;
  inode->ptr = pmm->alloc(sizeof(dir_t));
  inode->refcnt = 0;
  inode->type = VFILE_DIR;
  dir_t *new_dir = inode->ptr;
  new_dir->self = inode_ind;
  new_dir->pa = pa_dir_ind;
  pmm->free(resolvedpath);
  return 0;
}

int vfs_rmdir(const char *path) {
  // TODO:
  char fname[MAXNAMELEN];
  int inode_ind = path_parse(path);
  int pa_inode_ind = find_parent_dir(path, fname);
  assert(itable[inode_ind]->type == VFILE_DIR);
  dir_t *pre_dir = itable[inode_ind]->ptr;
  dir_t *pa_dir = itable[pa_inode_ind]->ptr;
  for (int i = 0; i < MAXDIRITEM; i++) {
    if (pre_dir->names[i]) pmm->free(pre_dir->names[i]);
    pre_dir->names[i] = NULL;
  }
  pmm->free(pre_dir);
  pre_dir = NULL;
  for (int i = 0; i < MAXDIRITEM; i++) {
    if (pa_dir->names[i] && strcmp(pa_dir->names[i], fname) == 0) {
      pmm->free(pa_dir->names[i]);
      pa_dir->names[i] = NULL;
    }
  }
  pmm->free(itable[inode_ind]);
  itable[inode_ind] = NULL;
  return 0;
}

int vfs_touch(const char *path) {
  // TODO:
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  filesystem_t *fs = find_mount_point_fs(resolvedpath);
  fs->iops->touch(resolvedpath);
  pmm->free(resolvedpath);
  return 0;
}

int vfs_rm(const char *path) {
  // TODO:
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  filesystem_t *fs = find_mount_point_fs(resolvedpath);
  fs->iops->rm(resolvedpath);
  pmm->free(resolvedpath);
  return 0;
}

int vfs_link(const char *oldpath, const char *newpath) {
  // TODO:
  // ?oldpath?inode number??newpath?
  // ??newpath???????
  return 0;
}
int vfs_unlink(const char *path) {
  // TODO:
  // unlink???path?????????
  // ?refcnt??????????link????inode?????
  return 0;
}

int vfs_open(const char *path, int flags) {
  // TODO:
  /*???????fd?????Lookup????inode????fd?inode???????fd
   */
  int new_fd = find_fd(cur_task);
  if (new_fd == -1) {
    log("open file failed, no free fd!");
    return -1;
  }
  char *resolvedpath = pmm->alloc(MAXPATHLEN);
  resolvedpath = realpath(path, resolvedpath);
  if (!resolvedpath) return -1;
  int inode_ind = path_parse(resolvedpath);
  inode_t *inode = itable[inode_ind];
  cur_task->fildes[new_fd]->inode = inode;
  inode->ops->open(cur_task->fildes[new_fd], flags);
  pmm->free(resolvedpath);
  return new_fd;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
  // TODO:
  // printf("reading~!\n");
  file_t *cur_file = cur_task->fildes[fd];
  if (cur_file) {
    return cur_file->inode->ops->read(cur_file, buf, nbyte);
  } else {
    log("wrong fd!");
    return -1;
  }
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
  // TODO:
  // printf("writing~!\n");
  file_t *cur_file = cur_task->fildes[fd];
  if (cur_file) {
    return cur_file->inode->ops->write(cur_file, buf, nbyte);
  } else {
    log("wrong fd!");
    return -1;
  }
}

off_t vfs_lseek(int fd, off_t offset, int whence) {
  // TODO:
  file_t *cur_file = cur_task->fildes[fd];
  cur_file->inode->ops->lseek(cur_file, offset, whence);
  return 0;
}

int vfs_close(int fd) {
  // TODO:
  file_t *tmp_file = cur_task->fildes[fd];
  tmp_file->inode->ops->close(tmp_file);
  pmm->free(tmp_file);
  return 0;
}

MODULE_DEF(vfs){
    .init = vfs_init,
    .access = vfs_access,
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .mkdir = vfs_mkdir,
    .rmdir = vfs_rmdir,
    .touch = vfs_touch,
    .rm = vfs_rm,
    .link = vfs_link,
    .unlink = vfs_unlink,
    .open = vfs_open,
    .read = vfs_read,
    .write = vfs_write,
    .lseek = vfs_lseek,
    .close = vfs_close,
};