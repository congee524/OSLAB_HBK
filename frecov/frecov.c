#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define DOS_DIR_SIZE 512

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

#define LASTDIR(num) ((num >> 6) & 1)
#define SEQDIR(num) (num & 0b11111)
#define HEAD(num) (num >>)
#define min(x, y) ((x) < (y) ? (x) : (y))

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
  char Name[8];        // 0x00 主文件名
  char ExtendName[3];  // 0x08 扩展名
  // char Name[11];
  BYTE Attr;                  // 0x0B 文件属性
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

typedef struct {
  BYTE SequeNumber;  // 0x00 序列号
  WORD Name1[5];     // 0x01 文件名的第1-5个Unicode码字符
  // char name1[10];
  BYTE Attr;       // 0x0b 属性标志 0xOF固定值
  BYTE Reserved1;  // 0x0c 保留未用
  BYTE CheckSum;   // 0x0d 短文件名检验和
  WORD Name2[6];   // 0x0e 文件名的第6-11个Unicode码字符
  // char name2[12];
  WORD Reserved2;  // 0x1A 保留未用 始终为0
  WORD Name3[2];   // 0x1c 文件名的第12-13个Unicode码字符
  // char name3[4];
} __attribute__((__packed__)) LFNEntry;

#define HEADSIZE 42

typedef struct {
  char bfType[2];
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
} __attribute__((__packed__)) BMPHeader;

// bmp info header
typedef struct {
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
} __attribute__((__packed__)) InfoHeader;

char *trname(LFNEntry *LFN, char *nbuffer) {
  char tmp_na;
  for (int i = 1; i >= 0; i--) {
    tmp_na = (char)LFN->Name3[i];
    // if (tmp_na >= 0x30 && tmp_na <= 0x7e) *(--nbuffer) = tmp_na;
    // if (tmp_na) *(--nbuffer) = tmp_na;
    *(--nbuffer) = tmp_na;
  }

  for (int i = 5; i >= 0; i--) {
    tmp_na = (char)LFN->Name2[i];
    // if (tmp_na >= 0x30 && tmp_na <= 0x7e) *(--nbuffer) = tmp_na;
    // if (tmp_na) *(--nbuffer) = tmp_na;
    *(--nbuffer) = tmp_na;
  }
  for (int i = 4; i >= 0; i--) {
    tmp_na = (char)LFN->Name1[i];
    // if (tmp_na >= 0x30 && tmp_na <= 0x7e) *(--nbuffer) = tmp_na;
    // if (tmp_na) *(--nbuffer) = tmp_na;
    *(--nbuffer) = tmp_na;
  }
  return nbuffer;
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("PLEASE INPUT FILENAME!\n");
    return 0;
  }
  int ca = 1;
  while (ca < argc) {
    BootEntry bootEntry;
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
    /*
    printf("Name = %s\n", bootEntry.BS_OEMName);

    printf("Bytes per Sector = %d\n", bps);
    printf("Sector per Cluster = %d\n", spc);

    printf("Reserved Sector Count = %d\n", bootEntry.BPBReservedSectorCount);
    printf("Number of FATs = %d\n", bootEntry.BPBNumberOfFATs);
    printf("Number of FAT sectors = %d\n", bootEntry.PBPSectorPerFAT);
*/
    int data_SecNum = bootEntry.BPBReservedSectorCount +
                      bootEntry.BPBNumberOfFATs * bootEntry.PBPSectorPerFAT;
    int rootDir_SecNum =
        data_SecNum + (bootEntry.BPBRootDirectoryCluster - 2) * spc;
    // printf("%d\n\n", (int)sizeof(LFNEntry));
    for (int pos = rootDir_SecNum * bps; pos < sb.st_size; pos += 32) {
      DirEntry *dirE = (DirEntry *)(addr + pos);
      if (dirE->Attr == 0x20 || dirE->Attr == 0x10) {
        if ((strncmp(dirE->ExtendName, "BMP", 3) != 0) &&
            (strncmp(dirE->ExtendName, "bmp", 3) != 0))
          continue;
        if (dirE->FileSize <= 0) continue;
        char tmp_name[32];
        memset(tmp_name, '\0', sizeof(tmp_name));

        if (dirE->Name[6] == '~' && dirE->Name[7] >= '1' &&
            dirE->Name[7] <= '5') {
          LFNEntry *LFN = (LFNEntry *)(addr + pos - 32);
          char name_buffer[32];
          memset(name_buffer, '\0', sizeof(name_buffer));
          char *nbuffer = &name_buffer[32];

          while (LASTDIR(LFN->SequeNumber) != 1) {
            LFN--;
          }
          while (LFN->Attr == 0x0f) {
            nbuffer = trname(LFN, nbuffer);
            LFN++;
          }
          memcpy(tmp_name, nbuffer, 32);
          if (tmp_name[0] < 0x30 || tmp_name[1] > 0x7a) continue;
          // printf("%s\n", tmp_name);

        } else {
          int i = 0;
          for (; i < 8 && dirE->Name[i] != 0x20 && dirE->Name[i] != 0; i++) {
            tmp_name[i] = dirE->Name[i];
          }
          tmp_name[i++] = '.';
          for (int j = 0;
               j < 3 && dirE->ExtendName[j] != 0x20 && dirE->ExtendName[j] != 0;
               j++) {
            tmp_name[i++] = dirE->ExtendName[j];
          }
        }

        int fcluster =
            dirE->FileStartClusterLow + (dirE->FileStartClusterHigh << 16);
        int Bmp_SecNum = data_SecNum + (fcluster - 2) * spc;
        FILE *outfp = fopen(tmp_name, "a");
        fwrite(addr + (Bmp_SecNum * bps), dirE->FileSize, 1, outfp);
        fclose(outfp);
        char cflags[50];
        memcpy(cflags, "sha1sum ", 8);
        memcpy(cflags + 8, tmp_name, sizeof(tmp_name));
        system(cflags);
      }
    }
  }
  char *cflags = "rm *.bmp *.BMP";
  system(cflags);
  return 0;
}

/*
int lname_cnt = SEQDIR(LFN->SequeNumber);
        // printf("lname_cnt %d: ", lname_cnt);
        char name_buffer[32];
        memset(name_buffer, '\0', sizeof(name_buffer));
        char *nbuffer = &name_buffer[31];
        nbuffer = trname(LFN, nbuffer);
        lname_cnt--;
        while (lname_cnt--) {
          pos += 32;
          LFN = (LFNEntry *)(addr + pos);
          nbuffer = trname(LFN, nbuffer);
        }
        if ((int)(&name_buffer[31] - nbuffer) > 8) printf("%s\n", nbuffer);
        printf("%d\n", (int)((&name_buffer[31] - nbuffer) / sizeof(char)));
*/