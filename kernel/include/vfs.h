#ifndef __VFS_H__
#define __VFS_H__

#include <kernel.h>

typedef struct vfile file_t;
typedef struct filesystem filesystem_t;
typedef struct fsops fsops_t;
typedef struct inode inode_t;
typedef struct inodeops inodeops_t;
typedef struct mount_point_table mptable;

typedef struct {
  void (*init)();
  int (*access)(const char *path, int mode);
  int (*mount)(const char *path, filesystem *fs);
  int (*unmount)(const char *path);
  int (*mkdir)(const char *path);
  int (*rmdir)(const char *path);
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
  uint64_t offset;
  inode_t *inode;
};

// 暂时三个，两个blkfs(ramdisk0\1),一个devfs
struct filesystem {
  const char *name;
  fsops_t *ops;
  device_t dev;
  inodeops_t inodeops;
};

enum FILETYPE { VFILE_FILE = 0, VFILE_DIR, VFILE_PIPE };

struct inode {
  int refcnt;  // 硬链接数 可能不作实现
  void *ptr;   // private data
  filesystem_t *fs;
  inodeops_t *ops;  // 在inode被创建时，由文件系统的实现赋值
                    // inode ops也是文件系统的一部分
  int type;         // 普通文件 目录 管道
  ssize_t fsize;    // 文件大小
};

struct fsops {
  // 具体文件系统API的实现
  void (*init)(struct filesystem *fs, const char *name, dev_t *dev);
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
  int (*link)(const char *name, inode_t *inode);
  int (*unlink)(const char *name);
  // 你可以自己设计readdir的功能
};

/*关于mount
https://landoflinux.com/linux_fstab.html
 */
struct mount_point_table {
  const char *name;        /*设备名称 ramdisk0之类的*/
  const char *mount_point; /*挂载点 */
  filesystem_t *fs;        /*文件系统类型*/
  // int options; /*设置选项 */
  // int dump; /*是否备份 */
  // int pass; /*如何使用fsck */
}

#endif