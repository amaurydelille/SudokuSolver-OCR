#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>
#include <string.h>
#include "files.h"

#define INPUTS 10
#define IMAGES 10

SDL_Surface* load_image2(const char* path) {
    SDL_Surface* temp = IMG_Load(path);
    SDL_Surface* surface = malloc(sizeof(SDL_Surface));
    surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(temp);

    if (surface == NULL)
        errx(EXIT_FAILURE, "%s, image non existante : %s", SDL_GetError(), path);

    return surface;
}

double pixel_to_grayscale2(Uint32 pixel_color, SDL_PixelFormat* format) {
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);
    return (r + g + b) / 255.0;
}

int isempty(double* image){
    double white = 0.0;
    double black = 0.0;

    for(size_t i = 0; i < 784; i++){
        if (image[i] == 0)
            black++;
        else
            white++;
    }

    if (white/black < 0.05)
        return 1;
    else
        return 0;
}

double* get_formated_image(SDL_Surface* surface) {
    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    Uint32* pixels = (Uint32*)surface->pixels;

    double* output = malloc(sizeof(double) * len);
    if (output == NULL) {
        errx(EXIT_FAILURE, "Failed to allocate memory for output");
    }

    int lock = SDL_LockSurface(surface);
    if (lock) {
        free(output);
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    }

    for (int i = 0; i < len; i++) {
        output[i] = pixel_to_grayscale2(pixels[i], format);
    }

    SDL_UnlockSurface(surface);

    return output;
}

double* path_to_input(char* path) {
    SDL_Surface* surface = load_image2(path);
    double* inputs = get_formated_image(surface);
    free(surface);
    return inputs;
}

void printdigit(double* image) {
    for(size_t i = 0; i < 28; i++){
        for(size_t j = 0; j < 28; j++){
            double pixel = image[i * 28 + j]; 
            if (pixel == 0)
                printf("  ");
            else if (pixel < 0.5)
                printf(". ");
            else
                printf("X ");
        }
        printf("\n");
    }
}

int* load_labels(const char* path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier : labels()");
        return NULL;
    }

    int* numbers = malloc(sizeof(int) * SAMPLES);
    if (numbers == NULL) {
        perror("Erreur d'allocation de mémoire");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < SAMPLES; i++) {
        if (fscanf(file, "%d", &numbers[i]) != 1) {
            fprintf(stderr, "Erreur de lecture ou fichier incomplet\n");
            free(numbers);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    return numbers;
}

double** load_train(const char* path) {
    FILE* file = NULL;
    double** images = malloc(sizeof(double*) * SAMPLES);
    if (images == NULL) {
        perror("Erreur d'allocation de mémoire");
        return NULL;
    }

    for (size_t i = 0; i < SAMPLES; i++) {
        char filename[1024]; 
        sprintf(filename, "%s/%zu.jpg", path, i+1); 

        file = fopen(filename, "r");
        if (file == NULL) {
            sprintf(filename, "%s/%zu.jpeg", path, i+1);
            file = fopen(filename, "r");
        }

        if (file == NULL) {
            sprintf(filename, "%s/%zu.png", path, i+1);
            file = fopen(filename, "r");
        }

        if (file == NULL) {
            perror("Erreur lors de l'ouverture du fichier d'image : Dataset");
            for (size_t j = 0; j < i; j++) {
                free(images[j]);
            }
            free(images);
            return NULL;
        }

        images[i] = malloc(sizeof(double) * 784); 
        if (images[i] == NULL) {
            perror("Erreur d'allocation de mémoire pour une image");
            fclose(file);
            for (size_t j = 0; j <= i; j++) {
                free(images[j]);
            }
            free(images);
            return NULL;
        }
        images[i] = path_to_input(filename);
        fclose(file);
    }

    return images;
}