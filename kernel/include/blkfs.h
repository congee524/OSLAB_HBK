#ifndef __BLKFS_H__
#define __BLKFS_H__
#include <common.h>
// 需要做一个该文件系统的inode，就像ext2那样，inode额外做吧！
typedef struct blkfs_inode blk_inode;
typedef struct BITMAP bit_map;
// 直接指针32个，每块128(0x80)字节，文件最大4096字节
// 最多32768(0x8000)个文件，最少1024(0x400)个文件
#define DIREPOINTNUM 32
#define BLOCK_SIZE 0x80
#define BLOCK_NUM 0x8000
#define MAX_FSIZE (DIREPOINTNUM * BLOCK_SIZE - 1)

struct BITMAP {
  uint8_t used : 1;
};

struct blkfs_inode {
  size_t fsize;                      //文件大小
  // mode 未实现
  uint32_t mode;                     // 文件的读写执行权限
  int link_num;                      // 文件的链接数
  int type;                          // 文件类型
  uint16_t ptr_point[DIREPOINTNUM];  // 文件内容指针
};

#endif