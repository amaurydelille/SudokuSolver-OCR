#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>
#include <err.h>
#include <stdlib.h>

#include "utils.h"
#include "crop-manager.h"
#include "cutter.h"

#include <string.h>

#define pi 3.1415926535

Intersection *FindIntersections(SDL_Surface *surface, unsigned int *normalSpace,
                                size_t vertLen, size_t horiLen) {
    unsigned int w = surface->w, h = surface->h;

    size_t len = vertLen * horiLen;
    Intersection *coords = calloc(len, sizeof(Intersection));

    // Fill the arrays with the intersections
    size_t j = 0;
    unsigned int x = 0, y = 0;
    for (size_t i = 0; i < w * h; i++) {
        if (normalSpace[i] >= 2 && !DontAdd(coords, x, y, j)) {
            coords[j].x = x;
            coords[j].y = y;
            j++;
        }
        x++;
        if (x == w) {
            x = 0;
            y++;
        }
    }

    size_t start = 0;
    for (size_t i = 1; i < len; i++) {
        size_t j = i;
        if (i % vertLen == 0) {
            start = i;
        }
        while (j > start && coords[j - 1].x > coords[j].x) {
            Intersection tmp = coords[j - 1];
            coords[j - 1] = coords[j];
            coords[j] = tmp;
            j--;
        }
    }

    vertLen = vertLen > 10 ? 10 : vertLen;
    horiLen = horiLen > 10 ? 10 : horiLen;

    /*
        printf("top-left:      (%u, %u)\n", coords[0].x, coords[0].y);
        printf("top-right:     (%u, %u)\n", coords[vertLen-1].x,
       coords[vertLen-1].y); printf("bottom-left:   (%u, %u)\n",
       coords[vertLen*horiLen-vertLen].x, coords[vertLen*horiLen-vertLen].y);
        printf("bottom-right:  (%u, %u)\n", coords[vertLen*horiLen-1].x,
       coords[vertLen*horiLen-1].y);
    */
    return coords;
}

int DontAdd(Intersection *coords, size_t x, size_t y, size_t nbIntersections) {
    /**
     * Check if a line is similar to an already existing one
     * Return 1 if the line already exist, 2 if a bigger one is found, 0 else
     */

    for (size_t i = 0; i < nbIntersections; i++) {
        Intersection iCoord = coords[i];

        int dx = abs((int) (iCoord.x) - (int) x);
        int dy = abs((int) (iCoord.y) - (int) y);

        if (dx <= 30 && dy <= 30) {
            return 1;
        }
    }
    return 0;
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

// Définition de la structure pour un élément
typedef struct PixelPosition {
    int x;
    int y;
    struct PixelPosition *next;
} PixelPosition;

// Définition de la structure pour la file d'attente
typedef struct Queue {
    PixelPosition *front;
    PixelPosition *rear;
} Queue;

// Créer un nouvel élément
PixelPosition *newPixelPosition(int x, int y) {
    PixelPosition *temp = (PixelPosition *) malloc(sizeof(PixelPosition));
    temp->x = x;
    temp->y = y;
    temp->next = NULL;
    return temp;
}

// Créer une file d'attente vide
Queue *createQueue() {
    Queue *q = (Queue *) malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}


// Ajouter un élément à la file d'attente
void enqueue(Queue *q, int x, int y) {
    PixelPosition *temp = newPixelPosition(x, y);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

// Retirer un élément de la file d'attente
PixelPosition *dequeue(Queue *q) {
    if (q->front == NULL)
        return NULL;
    PixelPosition *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;
    return temp;
}

// Vérifier si la file d'attente est vide
int isQueueEmpty(Queue *q) {
    return (q->front == NULL);
}

float getPercentage(SDL_Surface *surface, int numberPixels) {
    return (float) numberPixels / (float) (surface->w * surface->h);
}

long howManyPixelTouchIt(SDL_Surface *s, int x, int y) {
    SDL_Surface *surface = SDL_CreateRGBSurface(0, s->w, s->h, s->format->BitsPerPixel,
                                                s->format->Rmask, s->format->Gmask,
                                                s->format->Bmask, s->format->Amask);

    SDL_BlitSurface(s, NULL, surface, NULL);

    long res = 0;
    int w = surface->w;
    int h = surface->h;
    int top = y;
    int bottom = y;
    int left = x;
    int right = x;

    Uint32 *pixels = surface->pixels;
    SDL_PixelFormat *format = surface->format;
    pixels[y * w + x] = SDL_MapRGB(format, 0, 0, 0);

    Queue *q = createQueue();
    enqueue(q, x, y);
    while (!isQueueEmpty(q)) {
        PixelPosition *n = dequeue(q);
        res++;
        printf("Dequeue: %d, %d\n", n->x, n->y);
        if (n->x > 0 && n->x < w - 1 && n->y > 0 && n->y < h - 1) {
            for (int i = n->x - 1; i <= n->x + 1; ++i) {
                for (int j = n->y - 1; j <= n->y + 1; ++j) {
                    // printf("check x : %i, y : %i\n", i, j);
                    Uint32 pixel_color = pixels[j * w + i];
                    Uint8 r, g, b;
                    SDL_GetRGB(pixel_color, format, &r, &g, &b);
                    if (r != 0) {
                        enqueue(q, i, j);
                        pixels[j * w + i] = SDL_MapRGB(format, 0, 0, 0);
                        if (j < top) top = j;
                        if (j > bottom) bottom = j;
                        if (i < left) left = i;
                        if (i > right) right = i;
                    }

                }
            }
        }
        free(n);
    }
    free(q);
    return res;
}


void removeLittleStructures(SDL_Surface *surface) {
    int w = surface->w;
    int h = surface->h;
    Uint32 *pixels = surface->pixels;
    SDL_PixelFormat *format = surface->format;
    int x = 0;
    int y = 0;
    float p = 0;
    //printf("StartRemoving\n");
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);

            if (r != 0) {
                long res = howManyPixelTouchIt(surface, x, y);
                float temp = getPercentage(surface, (int) res);
                if (p != temp) {
                    printf("Res : %li /P : %f\n", res, p);
                    p = temp;
                }
                if (temp < 0.04) __removeWhitePixels(surface, pixels, format, w, h, x, y, 0, 0);
            }
        }
    }
}

void removeWhiteBorder(SDL_Surface *surface) {

    int w = surface->w;
    int h = surface->h;
    Uint32 *pixels = surface->pixels;
    SDL_PixelFormat *format = surface->format;
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < 3; ++y) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > 100) __removeWhitePixels(surface, pixels, format, w, h, x, y, 0, 50);
        }
    }
    for (int x = 0; x < w; ++x) {
        for (int y = h - 1; y > h - 4; --y) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > 100) __removeWhitePixels(surface, pixels, format, w, h, x, y, 0, 50);
        }
    }
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < 3; ++x) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > 100) __removeWhitePixels(surface, pixels, format, w, h, x, y, 0, 50);
        }
    }
    for (int y = 0; y < h; ++y) {
        for (int x = w - 1; x > w - 4; --x) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > 100) __removeWhitePixels(surface, pixels, format, w, h, x, y, 0, 50);
        }
    }
}

void makeItWhiteAndBlack(SDL_Surface *surface) {

    int w = surface->w;
    int h = surface->h;
    Uint32 *pixels = surface->pixels;
    SDL_PixelFormat *format = surface->format;
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            Uint32 pixel_color = pixels[y * w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel_color, format, &r, &g, &b);
            if (r > 200) pixels[y * w + x] = SDL_MapRGB(format, 255, 255, 255);
            else
                pixels[y * w + x] = SDL_MapRGB(format, 0, 0, 0);
        }
    }
}

SDL_Surface **CropSquares(SDL_Surface *surface, Intersection *coords,
                          size_t vertLen, size_t horiLen) {
    SDL_Surface **res = malloc(81 * sizeof(SDL_Surface * ));
    mkdir("squares", 0700);
    int i = 1;
    for (size_t y = 0; y < horiLen - 1; y++) {
        for (size_t x = 0; x < vertLen - 1; x++) {
            Intersection current = coords[y * vertLen + x];

            unsigned int squareWidth =
                    coords[y * vertLen + x + 1].x - current.x;
            unsigned int squareHeight =
                    coords[(y + 1) * vertLen + x].y - current.y;

            SDL_Surface *square =
                    CropSurface(surface, current, squareWidth, squareHeight);

            res[y * (vertLen - 1) + x] = square;
            char name[50];
            if (i < 10)
                sprintf(name, "%i.png", i % 10);
            else
                sprintf(name, "%i%i.png", i / 10, i % 10);


            char *newStr = malloc((strlen(name) + 15) *
                                  sizeof(char));
            strcpy(newStr, "squares/");
            strcat(newStr, name);

            removeWhiteBorder(square);
            printf("Number :%i\n", i);
            makeItWhiteAndBlack(square);
            //removeLittleStructures(square);
            IMG_SavePNG(square, newStr);
            free(newStr);
            SDL_FreeSurface(square);
            i++;
        }
    }
    return res;
}

SDL_Surface **ManualCrop(SDL_Surface *surface, Intersection *corners) {
    Intersection *coords = malloc(100 * sizeof(Intersection));

    Intersection *leftSide = FindPoints(corners[0], corners[2]);
    Intersection *rightSide = FindPoints(corners[1], corners[3]);

    for (size_t i = 0; i < 10; i++) {
        Intersection *line = FindPoints(leftSide[i], rightSide[i]);
        for (size_t j = 0; j < 10; j++) {
            coords[i * 10 + j] = line[j];
        }
        free(line);
    }

    free(leftSide);
    free(rightSide);
    return CropSquares(surface, coords, 10, 10);
}

Intersection *FindPoints(Intersection a, Intersection b) {
    Intersection *res = malloc(10 * sizeof(Intersection));
    res[0] = a;

    int dx = ((int) b.x - (int) a.x) / 9;
    int dy = ((int) b.y - (int) a.y) / 9;
    for (size_t i = 1; i < 9; i++) {
        res[i].x = res[i - 1].x + dx;
        res[i].y = res[i - 1].y + dy;
    }
    res[9] = b;
    return res;
}

SDL_Surface *CropSurface(SDL_Surface *surface, Intersection current, int width,
                         int height) {
    SDL_Surface *newSurface = SDL_CreateRGBSurface(
            surface->flags, width, height, surface->format->BitsPerPixel,
            surface->format->Rmask, surface->format->Gmask, surface->format->Bmask,
            surface->format->Amask);

    SDL_Rect rect = {current.x, current.y, width, height};
    SDL_BlitSurface(surface, &rect, newSurface, NULL);

    SDL_Surface *res = SDL_CreateRGBSurface(
            surface->flags, 28, 28, surface->format->BitsPerPixel,
            surface->format->Rmask, surface->format->Gmask, surface->format->Bmask,
            surface->format->Amask);

    StretchBlit(newSurface, res);
    SDL_FreeSurface(newSurface);

    return res;
}

void StretchBlit(SDL_Surface *src, SDL_Surface *dest) {
    SDL_Surface *img =
            SDL_CreateRGBSurface(SDL_SWSURFACE, dest->w, dest->h, 32, 0, 0, 0, 0);
    StretchLinear(src, img);
    SDL_BlitSurface(img, NULL, dest, NULL);
    SDL_FreeSurface(img);
}

void StretchLinear(SDL_Surface *src, SDL_Surface *dest) {
    double rx = dest->w * 1.0 / src->w;
    double ry = dest->h * 1.0 / src->h;

    for (int i = 0; i < dest->w; i++) {
        for (int j = 0; j < dest->h; j++) {
            unsigned char pix;
            double valx = i / rx;
            double valy = j / ry;
            int minx = (int) valx;
            int miny = (int) valy;
            int maxx = minx + 1;

            if (maxx >= src->w)
                maxx--;

            int maxy = miny + 1;
            if (maxy >= src->h)
                maxy--;

            double fx = valx - minx;
            double fy = valy - miny;

            for (int k = 0; k < 3; k++) {
                pix = (unsigned char) (GetPixelComp32(src, minx, miny, k) *
                                       (1 - fx) * (1 - fy) +
                                       GetPixelComp32(src, maxx, miny, k) * fx *
                                       (1 - fy) +
                                       GetPixelComp32(src, minx, maxy, k) *
                                       (1 - fx) * fy +
                                       GetPixelComp32(src, maxx, maxy, k) * fx *
                                       fy);
                PutPixelComp32(dest, i, j, k, pix);
            }
        }
    }
}

unsigned char GetPixelComp32(SDL_Surface *surface, int x, int y, int c) {
    unsigned char *p =
            ((unsigned char *) surface->pixels) + y * surface->pitch + x * 4;
    return p[c];
}

void PutPixelComp32(SDL_Surface *surface, int x, int y, int c,
                    unsigned char val) {
    unsigned char *p =
            ((unsigned char *) surface->pixels) + y * surface->pitch + x * 4;
    p[c] = val;
}
