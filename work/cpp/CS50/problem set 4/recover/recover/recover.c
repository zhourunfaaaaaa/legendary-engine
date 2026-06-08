#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LENGTH 512
 
int main(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        return 0;
    }

    uint8_t buffer[LENGTH]; 
    int counter = 0;
    FILE *outptr = NULL;
    char filename[8];

    while (fread(buffer, 1, LENGTH, fp) == LENGTH) {
        if (buffer[0] == 0xff &&  // 第 1 字节
            buffer[1] == 0xd8 &&  // 第 2 字节
            buffer[2] == 0xff &&  // 第 3 字节
            (buffer[3] & 0xf0) == 0xe0) {  // 第 4 字节，只关心高 4 位 
                if (outptr != NULL) {
                    fclose(outptr);
                }

                // 建新文件
                sprintf(filename, "%03i.jpg", counter);
                outptr = fopen(filename, "wb");
                counter++;
            }

            // 往当前JPEG写数据（跳过开头垃圾数据）
            if (outptr != NULL) {
                fwrite(buffer, LENGTH, 1, outptr);
            }
    }
    
    fclose(outptr);
    fclose(fp);

    return 0;
}