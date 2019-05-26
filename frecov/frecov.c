#include <stdio.h>
#include <stdlib.h>

typedef struct BootEntry {
  // __attribute__((packed)) means align according to the actual bytes occupied
  unsigned char BS_jmpBoot[3] __attribute__((packed));
  unsigned char BS_OEMName[8] __attribute__((packed));
  unsigned short BPB_BytsPerSec __attribute__((packed));
  unsigned char BPB_SecPerClus __attribute__((packed));
  unsigned short BPB_RsvdSecCnt __attribute__((packed));
  unsigned char BPB_NumFATs __attribute__((packed));
  unsigned short BPB_RootEntCnt __attribute__((packed));
  unsigned short BPB_TotSec16 __attribute__((packed));
  unsigned char BPB_Media __attribute__((packed));
  unsigned short BPB_FATSz16 __attribute__((packed));
  unsigned short BPB_SecPerTrk __attribute__((packed));
  unsigned short BPB_NumHeads __attribute__((packed));
  unsigned long BPB_HiddSec __attribute__((packed));
  unsigned long BPB_TotSec32 __attribute__((packed));
  unsigned long BPB_FATSz32 __attribute__((packed));
  unsigned short BPB_ExtFlags __attribute__((packed));
  unsigned short BPB_FSVer __attribute__((packed));
  unsigned long BPB_RootClus __attribute__((packed));
  unsigned short BPB_FSInfo __attribute__((packed));
  unsigned short BPB_BkBootSec __attribute__((packed));
  unsigned char BPB_Reserved[12] __attribute__((packed));
  unsigned char BS_DrvNum __attribute__((packed));
  unsigned char BS_Reserved1 __attribute__((packed));
  unsigned char BS_BootSig __attribute__((packed));
  unsigned long BS_VolID __attribute__((packed));
  unsigned char BS_VolLab[11] __attribute__((packed));
  unsigned char BS_FilSysType[8] __attribute__((packed));
} BootEntry;

typedef struct DirEntry {
  unsigned char DIR_Name[11] __attribute__((packed));
  unsigned char DIR_Attr __attribute__((packed));
  unsigned char DIR_NTRes __attribute__((packed));
  unsigned char DIR_CrtTimeTenth __attribute__((packed));
  unsigned short DIR_CrtTime __attribute__((packed));
  unsigned short DIR_CrtDate __attribute__((packed));
  unsigned short DIR_LstAccDate __attribute__((packed));
  unsigned short DIR_FstClusHI __attribute__((packed));
  unsigned short DIR_WrtTime __attribute__((packed));
  unsigned short DIR_WrtDate __attribute__((packed));
  unsigned short DIR_FstClusLO __attribute__((packed));
  unsigned long DIR_FileSize __attribute__((packed));
} DirEntry;

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