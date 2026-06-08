#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: ./volume input.wav output.wav factor\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }

    FILE *output = fopen(argv[2], "wb");
    if (output == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }
    const int HEADER_SIZE = 44;
    uint8_t header[HEADER_SIZE];
    fread(header, HEADER_SIZE, 1, input);
    fwrite(header, HEADER_SIZE, 1, output);
    
    int16_t buffer;
    float factor = atof(argv[3]); // atof = "ascii to float"
    while (fread(&buffer, sizeof(int16_t), 1, input)) {
        buffer *= factor;
        fwrite(&buffer, sizeof(int16_t), 1, output);
    }
    fclose(input);
    fclose(output);
    return 0;
}