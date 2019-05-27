#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define DOS_DIR_SIZE 512

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef struct {
  BYTE BS_jmpBoot[3];             // 0X00 跳跃指令
  BYTE BS_OEMName[8];             // 0X03 OEM代码
  WORD BPBBytesPerSector;         // 0x0B 每扇区字节数
  BYTE BPBSectorsPerClusters;     // 0x0D 每簇扇区数
  WORD BPBReservedSectorCount;    // 0x0E 保留扇区数
  BYTE BPBNumberOfFATs;           // 0x10 FAT表个数
  WORD BPBFAT32Unused1;           // 0x11 未用
  WORD BPBFAT32Unused2;           // 0x13 未用
  BYTE BPBMedia;                  // 0x15 介质描述符
  WORD BPBFAT32Unused3;           // 0x16 未用
  WORD BPBSectorPerTruck;         // 0x18 每磁道扇区数
  WORD BPBNumberOfHeads;          // 0x1A 磁头数
  DWORD BPBHideSector;            // 0x1C 隐藏扇区
  DWORD BPBTotalSector;           // 0x20 该分区总扇区数
  DWORD PBPSectorPerFAT;          // 0x24 每FAT扇区数
  WORD BPBExtFlag;                // 0x28 标记
  WORD BPBRevesion;               // 0x2A 版本
  DWORD BPBRootDirectoryCluster;  // 0x2C 根目录首簇号
  WORD BPBFSInfo;                 // 0x30 文件系统信息扇区号
  WORD BPBBackupDBRSector;        // 0x32 DBR备份扇区号
  BYTE BPBReserved[12];           // 0x34 保留
  BYTE BPBBIOSDriveNumber;        // 0x40 BIOS驱动器号
  BYTE BPBFAT32Unused4;           // 0x41 未用
  BYTE BPBExBootFlag;             // 0x42 扩展引导标记
  DWORD BPBVolumeID;              // 0x43 卷序列号
  BYTE BPBVolumeName[11];         // 0x47 卷标
  BYTE BPBFileSystem[8];          // 0x52 文件系统类型
} __attribute__((__packed__)) BootEntry;

typedef struct {
  char Name[8];               // 0x00 主文件名
  char ExtendName[3];         // 0x08 扩展名
  BYTE Attribute;             // 0x0B 文件属性
  BYTE Reserved;              // 0x0C 未用
  BYTE FileCreateTimeSecond;  // 0x0D 文件创建时间精确到秒
  WORD FileCreateTime;        // 0x0E 文件创建时间
  WORD FileCreateDate;        // 0x10 文件创建日期
  WORD FileAccessDate;        // 0x12 文件访问日期
  WORD FileStartClusterHigh;  // 0x14 文件起始簇号高位
  WORD FileModifyTime;        // 0x16 修改时间
  WORD FileModifyDate;        // 0x18 修改日期
  WORD FileStartClusterLow;   // 0x1A 文件起始簇号低位
  DWORD FileSize;             // 0x1C 文件大小（Byte为单位）
} __attribute__((__packed__)) DirEntry;

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("PLEASE INPUT FILENAME!\n");
    return 0;
  }
  int ca = 1;
  while (ca < argc) {
    BootEntry bootEntry;
    DirEntry DirEntry;
    char *addr;
    int fd;
    struct stat sb;
    // size_t length;
    // ssize_t s;

    char *filename = argv[ca++];
    fd = open(filename, O_RDONLY);
    if ((fd = open(filename, O_RDONLY)) == -1) {
      printf("Wrong filename %s!\n", filename);
      return 1;
    }
    if (fstat(fd, &sb) == -1) {
      printf("fstat go wrong!\n");
      return 1;
    }
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
      printf("mmap go wrong!\n");
      return 1;
    }
    // read DBR, get BPB parameters
    memcpy(&bootEntry, addr, sizeof(bootEntry));
    int bps = bootEntry.BPBBytesPerSector;      // 每扇区字节
    int spc = bootEntry.BPBSectorsPerClusters;  // 每簇扇区
    printf("Name = %s\n", bootEntry.BS_OEMName);
    printf("Bytes per Sector = %d\n", bps);
    printf("Sector per Cluster = %d\n", spc);

    printf("Reserved Sector Count = %d\n", bootEntry.BPBReservedSectorCount);
    printf("Number of FATs = %d\n", bootEntry.BPBNumberOfFATs);
    printf("Number of FAT sectors = %ld\n", bootEntry.PBPSectorPerFAT);
    int FAT_ind[2];
    // 保留扇区结束后即第一个FAT表,
    // 第二个FAT表还需加上第一个FAT表的扇区数
    FAT_ind[0] = bootEntry.BPBReservedSectorCount;
    FAT_ind[1] = bootEntry.BPBReservedSectorCount + bootEntry.PBPSectorPerFAT;
    printf("FAT1 : %d\n", FAT_ind[0]);
    printf("FAT2 : %d\n", FAT_ind[1]);
  }
  return 0;
}

/*
#define HEADSIZE 42

typedef struct BMPHEADER {
  char bfType[2];
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
} BMPHeader;

// bmp info header
typedef struct BMPInfoHeader {
  unsigned int biSize;
  int biWidth;
  int biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  int biXPelsPerMeter;
  int biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
} InfoHeader;

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("PLEASE INPUT FILENAME!\n");
    return 0;
  }
  int ca = 1;
  while (ca < argc) {
    // read file
    FILE *fp;
    char *filename = argv[ca++];
    fp = fopen(filename, "r");
    if (!fp) {
      printf("Wrong filename %s!\n", filename);
      fclose(fp);
      return 1;
    }
    fseek(fp, 0L, SEEK_END);
    int fsize = ftell(fp);
    int incre = 0;
    fseek(fp, 0L, SEEK_SET);

    // search file
    while (ftell(fp) < fsize) {
      BMPHeader head;
      fseek(fp, 1, SEEK_CUR);
      fread(&head, sizeof(BMPHeader), 1, fp);
      if ((head.bfType[0] != 'B') || (head.bfType[1] != 'M')) continue;
      char outfilename[32];
      snprintf(outfilename, 32, "%s_%d.bmp", filename, ++incre);

      FILE *outfp;
      outfp = fopen(outfilename, "a");
      char bmptmp[head.bfSize + head.bfOffBits];
      fread(bmptmp, sizeof(bmptmp), 1, fp);
      fwrite(&head, sizeof(head), 1, outfp);
      fwrite(bmptmp, sizeof(bmptmp), 1, outfp);
      fclose(outfp);
    }
    fclose(fp);
  }
  return 0;
}

*/