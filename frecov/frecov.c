#include <stdio.h>
#include <stdlib.h>

typedef struct BMPHEADER {
  char bfType[2];
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
} BMPHeader;

/*bmp info header*/
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
    fseek(fp, 0, SEEK_SET);

    // search file
    while (ftell(fp) < fsize) {
      BMPHeader head;
      fseek(fp, 1, SEEK_CUR);
      fread(&head, sizeof(BMPHeader), 1, fp);
      if ((head.bfType[0] != 'B') || (head.bfType[1] != 'M') ||
          (head.bfReserved1 != 0) || (head.bfReserved2 != 0))
        continue;
      char outfilename[32];
      snprintf(outfilename, 32, "%s_%d.bmp", filename, ++incre);

      FILE *outfp;
      outfp = fopen(outfilename, "a");
      char bmptmp[head.bfSize + head.bfOffBits];
      fread(bmptmp, sizeof(bmptmp), 1, fp);
      fwrite(bmptmp, sizeof(bmptmp), 1, outfp);
      fclose(outfp);
    }
    fclose(fp);
  }
  return 0;
}
