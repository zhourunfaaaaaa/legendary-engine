#include "helpers.h"
#include <math.h>

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int avg = round((image[i][j].rgbtBlue + image[i][j].rgbtGreen + image[i][j].rgbtRed)/3.0);
            image[i][j].rgbtBlue = image[i][j].rgbtGreen = image[i][j].rgbtRed = avg;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width/2; j++) {
            RGBTRIPLE tmp = image[i][j];
            image[i][j] = image[i][width-j-1];
            image[i][width-j-1] = tmp;
        }
    }
    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            copy[i][j] = image[i][j];
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sumR = 0,sumG = 0, sumB = 0;
            int count = 0;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && nj >= 0 && ni < height && nj < width) {
                        sumR += copy[ni][nj].rgbtRed;
                        sumG += copy[ni][nj].rgbtGreen;
                        sumB += copy[ni][nj].rgbtBlue;
                        count++;
                    }
                }
            }
            image[i][j].rgbtRed   = round((float)sumR / count);
            image[i][j].rgbtGreen = round((float)sumG / count);
            image[i][j].rgbtBlue  = round((float)sumB / count);
        }
    }
    
    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
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

    // 复制原图
    RGBTRIPLE copy[height][width];
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            copy[i][j] = image[i][j];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int gxR = 0, gyR = 0;
            int gxG = 0, gyG = 0;
            int gxB = 0, gyB = 0;

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni < 0 || ni >= height || nj < 0 || nj >= width)
                        continue;

                    int kx = Gx[di + 1][dj + 1];
                    int ky = Gy[di + 1][dj + 1];

                    gxR += copy[ni][nj].rgbtRed   * kx;
                    gyR += copy[ni][nj].rgbtRed   * ky;
                    gxG += copy[ni][nj].rgbtGreen * kx;
                    gyG += copy[ni][nj].rgbtGreen * ky;
                    gxB += copy[ni][nj].rgbtBlue  * kx;
                    gyB += copy[ni][nj].rgbtBlue  * ky;
                }
            }

            int newR = round(sqrt(gxR * gxR + gyR * gyR));
            int newG = round(sqrt(gxG * gxG + gyG * gyG));
            int newB = round(sqrt(gxB * gxB + gyB * gyB));

            image[i][j].rgbtRed   = (newR > 255) ? 255 : newR;
            image[i][j].rgbtGreen = (newG > 255) ? 255 : newG;
            image[i][j].rgbtBlue  = (newB > 255) ? 255 : newB;
        }
    }
    return;
}
