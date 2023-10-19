#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main() {
    int width, height, bpp;
    unsigned char* img = stbi_load("input.bmp", &width, &height, &bpp, 1);
    if (img == NULL) {
        printf("Erro ao carregar a imagem!\n");
        return 1;
    }

    unsigned char* output = malloc(width * height);

    int Gx[3][3] = {
        {-1, 0, 1},
        {-1, 0, 1},
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -1, -1},
        {0, 0, 0},
        {1, 1, 1}
    };

    #pragma omp parallel for
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int px = 0, py = 0;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int pixel = img[(y + i) * width + (x + j)];
                    px += pixel * Gx[i + 1][j + 1];
                    py += pixel * Gy[i + 1][j + 1];
                }
            }

            output[y * width + x] = sqrt(px * px + py * py);
        }
    }

    stbi_write_bmp("output.bmp", width, height, 1, output);

    free(img);
    free(output);

    printf("Imagem processada com sucesso!\n");

    return 0;
}
