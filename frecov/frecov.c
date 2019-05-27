#include <frecov.h>

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
    printf("Name = %s\n", bootEntry.BS_OEMName);
    printf("Bytes per Sector = %d\n", bps);
    printf("Sector per Cluster = %d\n", spc);

    printf("Reserved Sector Count = %d\n", bootEntry.BPBReservedSectorCount);
    printf("Number of FATs = %d\n", bootEntry.BPBNumberOfFATs);
    printf("Number of FAT sectors = %d\n", bootEntry.PBPSectorPerFAT);

    int data_SecNum = bootEntry.BPBReservedSectorCount +
                      bootEntry.BPBNumberOfFATs * bootEntry.PBPSectorPerFAT;
    int rootDir_SecNum =
        data_SecNum + (bootEntry.BPBRootDirectoryCluster - 2) * spc;

    for (int pos = rootDir_SecNum * bps; pos < sb.st_size; pos += 32) {
      LFNEntry *LFN = (LFNEntry *)(addr + pos);
      if (LFN->Attr == 0x0f && LASTDIR(LFN->SequeNumber) == 1) {
        for (int i = 0; i < 10; i++) printf("%c", LFN->name1[i]);
        for (int i = 0; i < 12; i++) printf("%c", LFN->name2[i]);
        for (int i = 0; i < 4; i++) printf("%c", LFN->name3[i]);
        printf("\n");
      }
    }
  }
  return 0;
}