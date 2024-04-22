#ifndef TREATMENT_H
#define TREATMENT_H


#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    int middle;
    int rangeLeft;
    int rangeRight;
} range;
typedef struct {
    int histogram[256];
} histo;

void surface_to_grayscale(SDL_Surface *surface);


void enhance_contrast(SDL_Surface *surface);


range GetHistogram(histo his, int w, int h, char *filename, int saveHistogram);

void removeRangeFromSurface(SDL_Surface *surface, int colorMax, int rangePlus, int rangeMinus);

void removewhitepixel(SDL_Surface *surface, int minValue);

int applyTreatment(char *filePath, char *outputPath, int withHistogram);

void resizeImageWrapper(const char *imagePath, const char *outputPath, int newWidth, int newHeight);

#endif
