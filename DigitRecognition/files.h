#pragma once

#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define IMAGE_WIDTH 28
#define IMAGE_HEIGHT 28
#define IMAGE_SIZE 784
#define DIRPATH "training"
#define SAMPLES 45000

SDL_Surface *load_image2(const char *path);

double pixel_to_grayscale2(Uint32 pixel_color, SDL_PixelFormat *format);

double *get_formated_image(SDL_Surface *surface);

double *path_to_input(char *path);

void printdigit(double *image);

int isempty(double *image);

int *load_labels(const char *path);

double **load_train(const char *path);