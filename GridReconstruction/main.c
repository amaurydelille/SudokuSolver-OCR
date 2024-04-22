#include "main.h"

#define largeGridSize 5
#define smallGridSize 2


int CreateGrid(char *savePath, char *firstGridPath, char *resolvedGridPath) {
    int w = 800;
    int h = 800;
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Init TTF
    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Window *window = SDL_CreateWindow("Reconstruction", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
    if (!surface) errx(EXIT_FAILURE, "Cannot create surface in CreateGrid !\n");

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) errx(EXIT_FAILURE, "Cannot create texture in CreateGrid !\n");

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);


    int smallWidth = w / 3;
    int smallHeight = w / 3;
    for (int i = 0; i < 4; ++i) {
        for (int j = -largeGridSize / 2; j < largeGridSize / 2; ++j) {
            SDL_RenderDrawLine(renderer, i * smallWidth + j, 0, i * smallWidth + j, h);
            SDL_RenderDrawLine(renderer, 0, i * smallHeight + j, w, i * smallHeight + j);
        }
        for (int k = 1; k < 3; ++k) {
            for (int j = -smallGridSize / 2; j < smallGridSize / 2; ++j) {

                SDL_RenderDrawLine(renderer, (i - 1) * smallWidth + (k * smallWidth / 3) + j, 0,
                                   (i - 1) * smallWidth + (k * smallWidth / 3) + j, h);
                SDL_RenderDrawLine(renderer, 0, (i - 1) * smallHeight + (k * smallHeight / 3) + j,
                                   w, (i - 1) * smallHeight + (k * smallHeight / 3) + j);
            }
        }
    }

    SDL_Color numberColor = {0, 0, 0, 255};

    TTF_Font *font = TTF_OpenFont("arial.ttf", w / 20);
    if (!font) errx(EXIT_FAILURE, "Cannot open font in GridReconstruction");

    FILE *fptr;

// Open a file in read mode
    fptr = fopen(firstGridPath, "r");
    if (!fptr) errx(EXIT_FAILURE, "Cannot open the file %s", firstGridPath);

    char myString[50];

    int res[9][9] = {0};
    int i = 0;
    while (fgets(myString, 50, fptr)) {
        if (myString[0] != '\n') {
            for (int j = 0; j < 9; ++j) {
                if (myString[j + (j / 3)] == '.')
                    res[i][j] = 0;
                else
                    res[i][j] = (int) myString[j + (j / 3)] - '0';
            }
            i++;
        }
    }
    fclose(fptr);




    // place numbers
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {

            if (res[j][i] != 0) {
                char text[20];
                sprintf(text, "%i", res[j][i]);
                SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, numberColor);
                if (!textSurface) errx(EXIT_FAILURE, "Cannot create textSurface in GridReconstruction");
                SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, textSurface), NULL,
                               &(SDL_Rect) {(w / 9) / 4 + i * (w / 9), (h / 9) / 4 + j * (h / 9), w / 20, h / 20});

                SDL_FreeSurface(textSurface);
            }
        }
    }

    // result
    //
    //

    fptr = fopen(resolvedGridPath, "r");
    if (!fptr) errx(EXIT_FAILURE, "Cannot open the file %s", firstGridPath);

    int trueRes[9][9] = {0};
    i = 0;
    while (fgets(myString, 50, fptr)) {
        if (myString[0] != '\n') {
            for (int j = 0; j < 9; ++j) {
                if (myString[j + (j / 3)] == '.')
                    trueRes[i][j] = 0;
                else
                    trueRes[i][j] = (int) myString[j + (j / 3)] - '0';
            }
            i++;
        }
    }
    fclose(fptr);


    SDL_Color numberColor2 = {255, 0, 0, 255};

    // place numbers
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {

            if (res[j][i] == 0) {
                char text[20];
                sprintf(text, "%i", trueRes[j][i]);
                SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, numberColor2);
                if (!textSurface) errx(EXIT_FAILURE, "Cannot create textSurface in GridReconstruction");
                SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, textSurface), NULL,
                               &(SDL_Rect) {(w / 9) / 4 + i * (w / 9), (h / 9) / 4 + j * (h / 9), w / 20, h / 20});

                SDL_FreeSurface(textSurface);
            }
        }
    }


    SDL_RenderPresent(renderer);

    // Save
    SDL_Surface *savedSurface = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!savedSurface) {
        fprintf(stderr, "Unable to create savedSurface\n");
        errx(EXIT_FAILURE, "Unable to create savedSurface");
    }

    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, savedSurface->pixels, savedSurface->pitch);

    IMG_SaveJPG(savedSurface, savePath, 100);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(savedSurface);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    TTF_CloseFont(font);
    SDL_Quit();
    return EXIT_SUCCESS;
}

/*
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <first grid> <solved grid> <output_image>\n", argv[0]);
        return EXIT_FAILURE;
    }
    CreateGrid( argv[3], argv[1], argv[2]);

    return EXIT_SUCCESS;

}
*/
