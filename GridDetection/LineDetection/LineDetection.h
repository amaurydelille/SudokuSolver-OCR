#ifndef INTERFACE_H
#define INTERFACE_H

#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

typedef struct 
{
    double rho;  
    double theta;
} polar_line;

typedef struct
{
    polar_line *lines;
    int nb_lines;
} hough_lines;

uint32_t Pixel_x_y(SDL_Surface *img, int j, int i);

void draw2(SDL_Renderer* renderer, SDL_Texture* texture);

void sort(int n, int* ptr);

hough_lines* cv_hough(SDL_Surface* img);

hough_lines* hough_transform(SDL_Surface* img);

void drawLine2(SDL_Surface *Screen, int x0, int y0, int x1, int y1, uint32_t pixel);

int clamp(int value, int min, int max);

void Drawlines2(SDL_Surface *source, hough_lines *parameters);

void event_loop(SDL_Renderer* renderer, SDL_Texture* colored, SDL_Texture* grayscale);

SDL_Surface* load_image2(const char* path);



#endif
