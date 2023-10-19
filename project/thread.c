#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "lodepng.h"

#define NUM_THREADS 2

// Imagem em nível de cinza
uint8_t* grayscale_image = NULL;
int width, height;

// Matrizes para Gx, Gy e G
int* Gx = NULL;
int* Gy = NULL;
int* G = NULL;

// Função para calcular Gx
void* calculate_Gx(void* arg) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sum = grayscale_image[(y + 1) * width + (x - 1)] +
                      grayscale_image[(y + 1) * width + x] +
                      grayscale_image[(y + 1) * width + (x + 1)] -
                      (grayscale_image[(y - 1) * width + (x - 1)] +
                       grayscale_image[(y - 1) * width + x] +
                       grayscale_image[(y - 1) * width + (x + 1)]);

            Gx[y * width + x] = fmax(0, fmin(sum, 255));
        }
    }
    return NULL;
}

// Função para calcular Gy
void* calculate_Gy(void* arg) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sum = grayscale_image[(y - 1) * width + (x + 1)] +
                      grayscale_image[y * width + (x + 1)] +
                      grayscale_image[(y + 1) * width + (x + 1)] -
                      (grayscale_image[(y - 1) * width + (x - 1)] +
                       grayscale_image[y * width + (x - 1)] +
                       grayscale_image[(y + 1) * width + (x - 1)]);

            Gy[y * width + x] = fmax(0, fmin(sum, 255));
        }
    }
    return NULL;
}

int main() {
    // Carregue a imagem em nível de cinza usando a biblioteca lodepng
    unsigned error;
    error = lodepng_decode_file(&grayscale_image, &width, &height, "coins.png", LCT_GREY, 8);

    if (error) {
        fprintf(stderr, "Error %u: %s\n", error, lodepng_error_text(error));
        return 1;
    }

    // Aloque memória para Gx, Gy e G
    Gx = (int*)malloc(width * height * sizeof(int));
    Gy = (int*)malloc(width * height * sizeof(int));
    G = (int*)malloc(width * height * sizeof(int));

    // Crie as threads
    pthread_t threads[NUM_THREADS];

    pthread_create(&threads[0], NULL, calculate_Gx, NULL);
    pthread_create(&threads[1], NULL, calculate_Gy, NULL);

    // Aguarde as threads terminarem
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    // Combine Gx e Gy para obter G
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            G[y * width + x] = fmax(0, fmin(Gx[y * width + x] + Gy[y * width + x], 255));
        }
    }

    // Salve a imagem de saída
    unsigned char* output_image = (unsigned char*)G;
    error = lodepng_encode_file("output_image.png", output_image, width, height, LCT_GREY, 8);
    if (error) {
        fprintf(stderr, "Error %u: %s\n", error, lodepng_error_text(error));
    }

    // Libere a memória alocada
    free(grayscale_image);
    free(Gx);
    free(Gy);
    free(G);

    return 0;
}