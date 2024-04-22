#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "BlobDetection.h"
#include "Pixels.h"
#include "Morphology.h"
#include "FloodFill.h"

typedef struct {
    Point ul;
    Point ur;
    Point ll;
    Point lr;
} orderedPoints;


orderedPoints orderPoints(SDL_Surface *src) {
    // The top left (or lower right) will have the mimimum (or maximum) sum of x + y
    size_t minSum = src->w + src->h; // By default we'll say it's far right
    Point ul = {0, 0};

    size_t maxSum = 0;
    Point lr = {0, 0};

    int minDiff = src->w > src->h ? src->w : src->h;
    Point ll = {0, 0};

    int maxDiff = -1 * minDiff;
    Point ur = {0, 0};

    for (int y = 0; y < src->h; ++y) {
        for (int x = 0; x < src->w; ++x) {
            // We don't count non-white pixels
            if (I(src, x, y) != 0xff) continue;

            size_t sum = x + y;
            int diff = x - y;

            if (sum < minSum) {
                minSum = sum;
                ul.x = x;
                ul.y = y;
            }

            if (sum >= maxSum) {
                maxSum = sum;
                lr.x = x;
                lr.y = y;
            }

            if (diff < minDiff) {
                minDiff = diff;
                ll.x = x;
                ll.y = y;
            }

            if (diff > maxDiff) {
                maxDiff = diff;
                ur.x = x;
                ur.y = y;
            }
        }
    }

    orderedPoints result;
    result.ul = ul;
    result.lr = lr;
    result.ll = ll;
    result.ur = ur;

    return result;
}

orderedPoints findGridCorner(SDL_Surface *image) {
    dilate_in_place(image);

    BiggestBlob_result bb_res = findBiggestBlob(image);
    SDL_Surface *result = bb_res.res;

    erode_in_place(result);

    orderedPoints points = orderPoints(result);
    SDL_FreeSurface(result);

    return points;
}

void __removeWhitePixels(SDL_Surface *surface, Uint32 *pixels, SDL_PixelFormat *format, int w, int h, int xPixel,
                         int yPixel, Uint8 color, Uint8 researchColor) {

    //printf("Xpos : %i / Ypos : %i\n", xPixel, yPixel);
    SDL_LockSurface(surface);
    pixels[yPixel * w + xPixel] = SDL_MapRGB(format, color, color, color);
    SDL_UnlockSurface(surface);
    int left = (xPixel > 0);
    int right = (xPixel < w - 1);
    int top = (yPixel > 0);
    int bottom = (yPixel < h - 1);
    for (int x = xPixel - left; x <= xPixel + right; ++x) {
        for (int y = yPixel - top; y <= yPixel + bottom; ++y) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > researchColor) {
                __removeWhitePixels(surface, pixels, format, w, h, x, y, color, researchColor);
            }
        }
    }
}

void removeWhiteBorder(SDL_Surface *surface) {

    int w = surface->w;
    int h = surface->h;
    Uint32 *pixels = surface->pixels;
    SDL_PixelFormat *format = surface->format;
    for (int x = 0; x < surface->w; ++x) {
        Uint32 pixel_color = pixels[x];
        Uint8 r, g, b;
        SDL_GetRGB(pixel_color, format, &r, &g, &b);
        if (r > 100) __removeWhitePixels(surface, pixels, format, w, h, x, 0, 0, 50);
    }
}

SDL_Texture *resizeImage(SDL_Renderer *renderer, SDL_Surface *originalSurface, int new_width, int new_height) {

    // Convertir la surface en texture
    SDL_Texture *originalTexture = SDL_CreateTextureFromSurface(renderer, originalSurface);
    if (!originalTexture) {
        printf("Erreur lors de la création de la texture : %s\n", SDL_GetError());
        return NULL;
    }

    // Créer une nouvelle texture avec les dimensions souhaitées
    SDL_Texture *resizedTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                    new_width, new_height);
    if (!resizedTexture) {
        printf("Erreur lors de la création de la texture redimensionnée : %s\n", SDL_GetError());
        return NULL;
    }

    // Copier et redimensionner l'image dans la nouvelle texture
    SDL_SetRenderTarget(renderer, resizedTexture);
    SDL_RenderCopy(renderer, originalTexture, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL); // Remettre le rendu à la cible par défaut

    SDL_DestroyTexture(originalTexture); // Libérer l'ancienne texture

    return resizedTexture;
}


int saveTextureAsImage(SDL_Renderer *renderer, SDL_Texture *texture, const char *outputPath, int width, int height) {
    int result = -1;

    // Créer une surface SDL pour contenir les pixels de la texture
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) {
        printf("Erreur lors de la création de la surface: %s\n", SDL_GetError());
        return result;
    }

    // Lire les pixels de la texture et les copier dans la surface
    SDL_SetRenderTarget(renderer, texture);
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch) == 0) {
        // Sauvegarder la surface en tant qu'image PNG
        if (IMG_SavePNG(surface, outputPath) == 0) {
            result = 0; // Succès
        } else {
            printf("Erreur lors de la sauvegarde de l'image: %s\n", IMG_GetError());
        }
    } else {
        printf("Erreur lors de la lecture des pixels de la texture: %s\n", SDL_GetError());
    }

    // Nettoyer
    SDL_SetRenderTarget(renderer, NULL);
    SDL_FreeSurface(surface);

    return result;
}

void GridCutter(SDL_Surface *img) {
    int newWidth = 28;
    int newHeight = 28;
    int size = img->w / 9;
    SDL_Rect rect = {0, 0, size, size};
    SDL_Surface *dest = SDL_CreateRGBSurface(0, size, size, img->format->BitsPerPixel, img->format->Rmask,
                                             img->format->Gmask, img->format->Bmask, img->format->Amask);
    mkdir("./grid_numbers", 0700);

    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur lors de l'initialisation de SDL: %s\n", SDL_GetError());
        return;
    }

    // Initialisation de SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("Erreur lors de l'initialisation de SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return;
    }

    // Création de la fenêtre et du renderer
    SDL_Window *window = SDL_CreateWindow("Image Resizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, newWidth,
                                          newHeight, 0);
    if (!window) {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur lors de la création du renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return;
    }

    for (size_t x = 0; x < 9; x += 1) {
        for (size_t y = 0; y < 9; y += 1) {
            char buf[50];
            sprintf(buf, "%s/%ld%ld.png", "./grid_numbers", x, y);
            printf("%s\n",buf);
            rect.x = x * size;
            rect.y = y * size;
            SDL_BlitSurface(img, &rect, dest, NULL);
            removeWhiteBorder(dest);

            // Appel de la fonction resizeImage
            SDL_Texture *resizedImage = resizeImage(renderer, dest, newWidth, newHeight);
            if (!resizedImage) {
                printf("Erreur lors du redimensionnement de l'image\n");
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                IMG_Quit();
                SDL_Quit();
                return;
            }

            saveTextureAsImage(renderer, resizedImage, buf, newWidth, newHeight);

            SDL_DestroyTexture(resizedImage);
        }
    }
    SDL_FreeSurface(dest);
    // Nettoyage et renvoi de la texture redimensionnée
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

SDL_Surface *load_image(const char *path) {
    SDL_Surface *surf = IMG_Load(path);
    if (surf == NULL) errx(EXIT_FAILURE, "%s", SDL_GetError());
    return SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGB888, 0);
}


int main(int argc, char **argv) {
    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: image-file");
    SDL_Surface *surface = load_image(argv[1]);

    MorphologyClose(surface);
    MorphologyOpen(surface);

    orderedPoints points = findGridCorner(surface);
    floodFill(surface, points.ul, WHITE, BLACK);

    GridCutter(surface);
    /*for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            char buf[50];
            sprintf(buf, "%s/%d%d.png", "./grid_numbers", i, j);
            //resizeImageWrapper(buf, buf, 28, 28);
        }
    }*/
    SDL_FreeSurface(surface);
    SDL_Quit();
    return EXIT_SUCCESS;
}
