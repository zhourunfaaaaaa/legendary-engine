#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"

int main(int argc, char *argv[])
{
     if (argc != 4)
    {
        printf("Usage: ./filter -flag infile outfile\n");
        return 1;
    }

    FILE *inptr = fopen(argv[2], "rb");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", argv[2]);
        return 1;
    }

    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;

    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    int width = bi.biWidth;
    int height = abs(bi.biHeight);


    // 每行 padding = (4 - (width * 3) % 4) % 4
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;
    // 逐行读
    RGBTRIPLE image[height][width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fread(&image[i][j], sizeof(RGBTRIPLE), 1, inptr);
        }
        fseek(inptr, padding, SEEK_CUR); // 移动文件指针，SEEK_CUR表示从当前位置往后移动
    }

    if (strcmp(argv[1], "-g") == 0)
        grayscale(height, width, image);
    else if (strcmp(argv[1], "-r") == 0)
        reflect(height, width, image);
    else if (strcmp(argv[1], "-b") == 0)
        blur(height, width, image);
    else if (strcmp(argv[1], "-e") == 0)
        edges(height, width, image);

    FILE *outptr = fopen(argv[3], "wb");

    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fwrite(&image[i][j], sizeof(RGBTRIPLE), 1, outptr);
        }
        for (int k = 0; k < padding; k++) {
            fputc(0x00, outptr); // padding个位置写0，凑齐4的倍数
        }
    }

    fclose(inptr);
    fclose(outptr);

    return 0;
}