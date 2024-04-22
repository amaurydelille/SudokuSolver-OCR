#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "treatment.h"
#include<unistd.h>


Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat *format) {
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);
    Uint8 average = r * 0.299 + g * 0.587 + b * 0.114;
    return SDL_MapRGB(format, average, average, average);
}


void surface_to_grayscale(SDL_Surface *surface) {
    Uint32 *pixels = surface->pixels;
    int len = surface->w * surface->h;
    SDL_PixelFormat *format = surface->format;
    SDL_LockSurface(surface);
    for (int i = 0; i < len; ++i) {
        pixels[i] = pixel_to_grayscale(pixels[i], format);
    }
    SDL_UnlockSurface(surface);
}


void enhance_contrast(SDL_Surface *surface) {
    Uint8 minVal = 255, maxVal = 0;
    Uint32 *pixels = (Uint32 *) surface->pixels;

    for (int i = 0; i < surface->w * surface->h; i++) {
        Uint8 val;
        SDL_GetRGB(pixels[i], surface->format, &val, &val, &val);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    for (int i = 0; i < surface->w * surface->h; i++) {
        Uint8 val;
        SDL_GetRGB(pixels[i], surface->format, &val, &val, &val);
        Uint8 newVal = (Uint8) 255 * (val - minVal) / (maxVal - minVal);
        pixels[i] = SDL_MapRGB(surface->format, newVal, newVal, newVal);
    }
}

int findRangePlus(int histogram[256], int max) {
    int i = max;
    int actualValue = histogram[i];
    while (i < 256) {
        if (actualValue >= histogram[i]) actualValue = histogram[i];
        else break;
        i++;
    }
    return i - max;
}

int findRangeMinus(int histogram[256], int max) {
    int i = max;
    int actualValue = histogram[i];
    while (i > 0) {
        if (actualValue >= histogram[i]) actualValue = histogram[i];
        else break;
        i--;
    }
    return max - i;
}

int IsColorInRange(Uint8 color, int colorMax, int rangePlus, int rangeMinus) {
    return color >= (colorMax - rangeMinus) && color <= (colorMax + rangePlus);
}


histo returnHistogram(SDL_Surface *s) {
    histo h;

    SDL_LockSurface(s);
    Uint32 *pixels = (Uint32 *) s->pixels;

    for (int i = 0; i < 255; i++) {
        h.histogram[i] = 0;
    }
    for (int i = 0; i < s->w * s->h; i++) {
        Uint8 val;
        SDL_GetRGB(pixels[i], s->format, &val, &val, &val);
        h.histogram[val]++;
    }
    SDL_UnlockSurface(s);

    // to skip white pixels
    h.histogram[255] = 0;
    return h;
}

histo deleteOnHistogram(histo h, range r) {
    // printf("RangeLeft :%i\nRangeRight :%i\n", r.rangeLeft, r.rangeRight);
    for (int i = r.middle - r.rangeLeft; i < r.middle + r.rangeRight; ++i) {
        h.histogram[i] = 0;
    }
    return h;
}

range GetHistogram(histo his, int w, int h, char *filename, int saveHistogram) {

    for (int i = 2; i < 254; ++i) {
        his.histogram[i] = (his.histogram[i - 2] + his.histogram[i - 1] + his.histogram[i] + his.histogram[i + 1] +
                            his.histogram[i + 2]) / 5;
    }


    int max = 0;
    int colorMax = 0;
    for (int i = 0; i < 256; ++i) {
        if (max < his.histogram[i]) {
            max = his.histogram[i];
            colorMax = i;
        }
    }
    if (saveHistogram)printf("max : %i\nColorMax : %i\n", max, colorMax);


    int rangePlus = findRangePlus(his.histogram, colorMax);
    int rangeMinus = findRangeMinus(his.histogram, colorMax);
    if (saveHistogram) {
        SDL_Window *window = SDL_CreateWindow("SDL Line Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w,
                                              h,
                                              SDL_WINDOW_SHOWN);

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (!surface) {
            fprintf(stderr, "Unable to create surface\n");
            errx(EXIT_FAILURE, "Unable to create surface");
        }
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            fprintf(stderr, "Unable to create texture\n");
            errx(EXIT_FAILURE, "Unable to create texture");
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);


        printf("RangePlus : %i\nRangeMinus : %i\n", rangePlus, rangeMinus);
        float mult = (float) (h - 10) / (float) max;
        for (int i = 0; i < 256; ++i) {
            if (IsColorInRange(i, colorMax, rangePlus, rangeMinus)) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawLine(renderer, i * 2, h - 10, i * 2, (h - 10) - (int) (mult * (float) his.histogram[i]));
            SDL_SetRenderDrawColor(renderer, i, i, i, 255);
            SDL_Rect square = {i * 2, h - 10, 2, 10};
            SDL_RenderDrawRect(renderer, &square);
        }
        SDL_RenderPresent(renderer);

        SDL_Surface *savedSurface = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (!savedSurface) {
            fprintf(stderr, "Unable to create savedSurface\n");
            errx(EXIT_FAILURE, "Unable to create savedSurface");
        }

        SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, savedSurface->pixels, savedSurface->pitch);

        IMG_SaveJPG(savedSurface, filename, 100);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(savedSurface);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);

    }
    range r = {colorMax, rangeMinus, rangePlus};

    return r;
}


void removeRangeFromSurface(SDL_Surface *surface, int colorMax, int rangePlus, int rangeMinus) {
    SDL_LockSurface(surface);

    Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);

    Uint32 *pixels = (Uint32 *) surface->pixels;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = pixels[y * surface->pitch / 4 + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            if (IsColorInRange(r, colorMax, rangePlus, rangeMinus) &&
                IsColorInRange(g, colorMax, rangePlus, rangeMinus) &&
                IsColorInRange(b, colorMax, rangePlus, rangeMinus)) {
                pixels[y * surface->pitch / 4 + x] = white;
            }
        }
    }

    SDL_UnlockSurface(surface);
}


void removewhitepixel(SDL_Surface *surface, int minValue) {
    SDL_LockSurface(surface);

    Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);

    Uint32 *pixels = (Uint32 *) surface->pixels;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = pixels[y * surface->pitch / 4 + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            if (r > minValue) {
                pixels[y * surface->pitch / 4 + x] = white;
            }
        }
    }

    SDL_UnlockSurface(surface);
}

float noneWhitePixel(SDL_Surface *surface) {
    int res = 0;
    SDL_LockSurface(surface);

    Uint32 *pixels = (Uint32 *) surface->pixels;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = pixels[y * surface->pitch / 4 + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            if (r != 255) {
                res++;
            }
        }
    }

    SDL_UnlockSurface(surface);
    return res;
}

int repeatAgain(SDL_Surface *surface) {
    float pourcentage = (float) noneWhitePixel(surface) / (float) (surface->w * surface->h);
    // printf("pourcentage = %f\n", pourcentage);
    return pourcentage > 0.13;
}

int finalBlackAndWhite(SDL_Surface *surface) {

    Uint32 *pixels = surface->pixels;
    SDL_PixelFormat *format = surface->format;
    SDL_LockSurface(surface);
    int width = surface->w;
    int height = surface->h;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            Uint32 pixel_color = pixels[y * width + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > 250)
                pixels[y * width + x] = SDL_MapRGB(format, 0, 0, 0);
            else
                pixels[y * width + x] = SDL_MapRGB(format, 255, 255, 255);
        }
    }
    SDL_UnlockSurface(surface);
    return 0;
}


int applyTreatment(char *filePath, char *outputPath, int withHistogram) { // withHistogram = debug (0 false / 1 true)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(IMG_Load(filePath), SDL_PIXELFORMAT_RGB888, 0);
    if (!surface) {
        fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", filePath, IMG_GetError());
        IMG_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }


    if (withHistogram) {
        if (SDL_SaveBMP(surface, "imagetreat.jpeg") != 0) {
            fprintf(stderr, "Unable to save image to %s! SDL Error: %s\n", outputPath, SDL_GetError());
            SDL_FreeSurface(surface);
            IMG_Quit();
            SDL_Quit();
            return EXIT_FAILURE;
        }
    }

    surface_to_grayscale(surface);
    removewhitepixel(surface, 220);
    int h = 400;
    int w = 512;
    char filename[50];
    int iteration = 0;
    histo his = returnHistogram(surface);
    while (repeatAgain(surface)) {
        sprintf(filename, "histogram%i.jpg", iteration);

        range r = GetHistogram(his, w, h, filename, withHistogram);
        his = deleteOnHistogram(his, r);
        if (r.middle < 100) break;
        removeRangeFromSurface(surface, r.middle, r.rangeRight, r.rangeLeft);
        iteration++;
    }
    if (withHistogram) GetHistogram(his, w, h, "histogramfinal.jpg", withHistogram);

    finalBlackAndWhite(surface);


    if (SDL_SaveBMP(surface, outputPath) != 0) {
        fprintf(stderr, "Unable to save image to %s! SDL Error: %s\n", filename, SDL_GetError());
        SDL_FreeSurface(surface);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_FreeSurface(surface);
    printf("New image saved as %s\n", outputPath);
    IMG_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

