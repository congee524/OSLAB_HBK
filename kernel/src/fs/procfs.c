#include <common.h>
#include <dir.h>
#include <vfs.h>

extern inode_t *itable[];
extern int mptable_cnt;
extern mptable_t mptable[];

inodeops_t procfs_iops;

void mount_procfile(dir_t *proc_root_dir, void *ptr, size_t fsize, char *name) {
  int ind = find_inode_ind();
  itable[ind] = pmm->alloc(sizeof(struct inode));
  inode_t *inode = itable[ind];
  inode->refcnt = 0;
  inode->ptr = ptr;
  inode->type = VFILE_FILE;
  inode->fsize = fsize;
  inode->ops = &procfs_iops;
  int dir_ind = 0;
  for (dir_ind = 0; dir_ind < MAXDIRITEM; dir_ind++) {
    if (!proc_root_dir->names[dir_ind]) break;
  }
  proc_root_dir->names[dir_ind] = pmm->alloc(MAXNAMELEN);
  strcpy(proc_root_dir->names[dir_ind], name);
  proc_root_dir->inodes_ind[dir_ind] = ind;
  return;
}

/*======= procfs_fsops =======*/

void procfs_init(filesystem_t *fs, const char *name, device_t *dev) {
  // TODO:
  fs->dev = dev;
  fs->name = name;

  int mp_ind = 0;
  for (mp_ind = 0; mp_ind < mptable_cnt; mp_ind++) {
    if (mptable[mp_ind].fs == fs) break;
  }
  assert(mp_ind < mptable_cnt);

  int mp_dir_inode_ind = path_parse(mptable[mp_ind].mount_point);
  assert(itable[mp_dir_inode_ind]->type == VFILE_DIR);
  dir_t *proc_root_dir = itable[mp_dir_inode_ind]->ptr;

  mount_procfile(proc_root_dir, cur_pwd, sizeof(cur_pwd), "pwd");
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
  strcpy(buf, (char *)file->inode->ptr);
  return 126;
}

ssize_t procfs_iwrite(file_t *file, const char *buf, size_t size) { return -1; }

off_t procfs_ilseek(file_t *file, off_t offset, int whence) { return -1; }

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
