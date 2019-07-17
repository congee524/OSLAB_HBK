#ifndef __VFS_H__
#define __VFS_H__

#include <kernel.h>

typedef struct vfile file_t;
typedef struct filesystem filesystem_t;
typedef struct fsops fsops_t;
typedef struct inode inode_t;
typedef struct inodeops inodeops_t;
typedef struct mount_point_table mptable_t;

#define MAXMOUNTPOINT 16
#define MAXINODENUM 0x10000

int find_inode_ind();
filesystem_t *find_mount_point_fs(const char *path);

enum SEEKTYPE { SEEK_SET = 0, SEEK_CUR, SEEK_END };

typedef struct {
  void (*init)();
  int (*access)(const char *path, int mode);
  int (*mount)(const char *path, filesystem_t *fs);
  int (*unmount)(const char *path);
  int (*mkdir)(const char *path);
  int (*rmdir)(const char *path);
  int (*touch)(const char *path);
  int (*rm)(const char *path);
  int (*link)(const char *oldpath, const char *newpath);
  int (*unlink)(const char *path);
  int (*open)(const char *path, int flags);
  ssize_t (*read)(int fd, void *buf, size_t nbyte);
  ssize_t (*write)(int fd, void *buf, size_t nbyte);
  int (*exec)(const char *file, void *args[]);
  off_t (*lseek)(int fd, off_t offset, int whence);
  int (*close)(int fd);
} MODULE(vfs);

struct vfile {
  int refcnt;  // 引用次数
  uint32_t offset;
  inode_t *inode;
  int flags;
};

// 暂时三个，两个blkfs(ramdisk0\1),一个devfs
/* 关于inode_table，
 * 可以做一个全局的，也就是所有文件系统，不管挂载在哪里，都在一个
 * inode_table中找，当然这不符合常识，还可以每个文件系统中单独存
 * 一个inode_table，实际在lookup的时候，总是先解析mount
 * point，得到fs之后读inode_table
 * */
struct filesystem {
  const char *name;
  fsops_t *ops;
  device_t *dev;
  inodeops_t *iops;
  // 我现在的实现好像不需要inode table，目录项都指向指针了
};

enum FILETYPE { VFILE_FILE = 0, VFILE_DIR, VFILE_PIPE };

struct inode {
  int refcnt;  // 硬链接数 unlink link create需要关注
  void *ptr;   // private data
  filesystem_t *fs;
  inodeops_t *ops;  // 在inode被创建时，由文件系统的实现赋值
                    // inode ops也是文件系统的一部分
  int type;         // 普通文件 目录 管道
  ssize_t fsize;    // 文件大小
};

struct fsops {
  // 具体文件系统API的实现
  void (*init)(struct filesystem *fs, const char *name, device_t *dev);
  inode_t *(*lookup)(struct filesystem *fs, const char *path, int flags);
  int (*close)(inode_t *inode);
};

struct inodeops {
  int (*open)(file_t *file, int flags);
  int (*close)(file_t *file);
  ssize_t (*read)(file_t *file, char *buf, size_t size);
  ssize_t (*write)(file_t *file, const char *buf, size_t size);
  off_t (*lseek)(file_t *file, off_t offset, int whence);
  int (*mkdir)(const char *name);
  int (*rmdir)(const char *name);
  int (*touch)(const char *name);
  int (*rm)(const char *name);
  int (*link)(const char *name, inode_t *inode);
  int (*unlink)(const char *name);
  // 你可以自己设计readdir的功能
};

/*关于mount
https://landoflinux.com/linux_fstab.html
https://blog.51cto.com/lspgyy/1297432
 */
struct mount_point_table {
  char *mount_point; /*挂载点 */
  filesystem_t *fs;  /*文件系统类型*/
  // int options; /*设置选项 */
  // int dump; /*是否备份 */
  // int pass; /*如何使用fsck */
};

#endif