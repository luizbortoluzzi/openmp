#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

int main() {
    FILE *fp = fopen("input.bmp", "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open image file!\n");
        return 1;
    }

    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);

    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    if (fileHeader.bfType != 0x4D42) {
        fprintf(stderr, "Error: Not a BMP file!\n");
        fclose(fp);
        return 1;
    }

    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;
    int stride = (width * 3 + 3) & ~3;
    int padding = stride - width * 3;

    unsigned char *pixels = malloc(stride * height);
    fseek(fp, fileHeader.bfOffBits, SEEK_SET);
    fread(pixels, stride * height, 1, fp);
    fclose(fp);

    unsigned char *output = malloc(stride * height);
    memcpy(output, pixels, stride * height);

    // Conversão para tons de cinza
    #pragma omp parallel for
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * stride + x * 3;
            int r = pixels[index];
            int g = pixels[index + 1];
            int b = pixels[index + 2];
    
            int gray = (r + g + b) / 3;
            pixels[index] = gray;
            pixels[index + 1] = gray;
            pixels[index + 2] = gray;
        }
    }

    #pragma omp parallel for
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int sumX[3] = {0, 0, 0};
            int sumY[3] = {0, 0, 0};

            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int index = (y + i) * stride + (x + j) * 3;
                    int r = pixels[index];
                    int g = pixels[index + 1];
                    int b = pixels[index + 2];

                    int kernelX = j == 0 ? 0 : j == -1 ? -1 : 1;
                    int kernelY = i == 0 ? 0 : i == -1 ? -1 : 1;

                    sumX[0] += kernelX * r;
                    sumX[1] += kernelX * g;
                    sumX[2] += kernelX * b;

                    sumY[0] += kernelY * r;
                    sumY[1] += kernelY * g;
                    sumY[2] += kernelY * b;
                }
            }

            int index = y * stride + x * 3;
            for (int i = 0; i < 3; ++i) {
                int val = sqrt(sumX[i] * sumX[i] + sumY[i] * sumY[i]);
                output[index + i] = val > 255 ? 255 : val;
            }
        }
    }

    fp = fopen("output.bmp", "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open output file!\n");
        free(pixels);
        free(output);
        return 1;
    }

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(output, stride * height, 1, fp);
    fclose(fp);

    free(pixels);
    free(output);

    printf("Image processed successfully!\n");

    return 0;
}
