#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "helpers.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: ./edges_demo -x|-y infile outfile\n");
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

    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    RGBTRIPLE image[height][width];
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
            fread(&image[i][j], sizeof(RGBTRIPLE), 1, inptr);
        fseek(inptr, padding, SEEK_CUR);
    }
    fclose(inptr);

    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    RGBTRIPLE copy[height][width];
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            copy[i][j] = image[i][j];

    int onlyGx = (strcmp(argv[1], "-x") == 0);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sumR = 0, sumG = 0, sumB = 0;

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni < 0 || ni >= height || nj < 0 || nj >= width)
                        continue;

                    int k = onlyGx ? Gx[di + 1][dj + 1] : Gy[di + 1][dj + 1];

                    sumR += copy[ni][nj].rgbtRed   * k;
                    sumG += copy[ni][nj].rgbtGreen * k;
                    sumB += copy[ni][nj].rgbtBlue  * k;
                }
            }

            // 取绝对值，不然负数显示不了
            if (onlyGx)
            {
                // Gx: 负值→取绝对值，这样左右边缘都能看到
                sumR = abs(sumR);
                sumG = abs(sumG);
                sumB = abs(sumB);
            }
            else
            {
                // Gy: 负值→取绝对值
                sumR = abs(sumR);
                sumG = abs(sumG);
                sumB = abs(sumB);
            }

            image[i][j].rgbtRed   = (sumR > 255) ? 255 : sumR;
            image[i][j].rgbtGreen = (sumG > 255) ? 255 : sumG;
            image[i][j].rgbtBlue  = (sumB > 255) ? 255 : sumB;
        }
    }

    FILE *outptr = fopen(argv[3], "wb");
    if (outptr == NULL)
    {
        printf("Could not open %s.\n", argv[3]);
        return 1;
    }

    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
            fwrite(&image[i][j], sizeof(RGBTRIPLE), 1, outptr);
        for (int k = 0; k < padding; k++)
            fputc(0x00, outptr);
    }

    fclose(outptr);
    return 0;
}
