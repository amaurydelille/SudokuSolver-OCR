#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <dirent.h>

#define PI 3.14159265359
#define MAX_ANGLE 1
#define MIN_FONT_SIZE 18
#define MAX_FONT_SIZE 23
#define DECAL_MAX 5
#define PATH "../DigitRecognition/Dataset2/"


int nb_fonts = 0;

// print a table of character with a size of len
void printTable(char *table, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%c,", table[i]);
    }
    printf("\n");
}

// print a table that contain fonts
void printFontTable(char *table, size_t nbFonts) {
    size_t i = 0;
    size_t actualFont = 0;
    while (nbFonts > actualFont) {
        while (table[i] != '\0') {
            printf("%c,", table[i]);
            i++;
        }
        actualFont++;
        i++;
        printf("//");
    }
    printf("\n");
}

// return the length of a char table.
// The startIndex notice where to start to count. A string finish with a '\0'
size_t getStringLength(char *string, size_t startIndex) {
    size_t len = 0;
    while (string[len + startIndex] != '\0')
        len++;
    return len;
}

// read the directory "fonts/" return a char* pointer that contain all the fonts separated by '\0' character.
char *readFontsName() {
    unsigned long nbCharacters = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("fonts/");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] != '.') {
                nbCharacters += strlen(dir->d_name);
                nb_fonts++;
            }
        }
        closedir(d);
    }
    size_t len = nbCharacters + 1 * nb_fonts;
    // each font will be stock in one line like this : a,r,i,a,l,.,t,t,f,\0,o,t,h,e,r,.,t,t,f,\0
    char *font = malloc(len);

    d = opendir("fonts/");
    int previousI = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] != '.') {
                for (size_t i = 0; i < strlen(dir->d_name); ++i) {
                    font[previousI] = dir->d_name[i];
                    previousI += 1;
                }
                font[previousI] = '\0';
                previousI += 1;
            }
        }
        closedir(d);
    }
    // uncomment code if you want to discover how it's work (i'm proud of myself so do it)
    //  printTable(font, len);
    return font;
}

// return a black pixel color
Uint32 blackColor(SDL_PixelFormat *format) {
    return SDL_MapRGB(format, 255, 255, 255);
}

// return a white pixel color
Uint32 whiteColor(SDL_PixelFormat *format) {
    return SDL_MapRGB(format, 0, 0, 0);
}

// rotate a new surface by angle degrees
SDL_Surface *rotate(SDL_Surface *surface, int angle) {
    SDL_LockSurface(surface);

    int w = surface->w;
    int h = surface->h;
    SDL_PixelFormat *format = surface->format;

    SDL_Surface *newSurface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);

    // Fill the surface with a white background
    SDL_FillRect(newSurface, NULL, SDL_MapRGB(format, 0, 0, 0));

    Uint32 *newPixels = (Uint32 *) newSurface->pixels;

    Uint32 *pixels = (Uint32 *) surface->pixels;

    // Rotate the text surface
    // printf("Angle : %i\n", angle);
    double alpha = ((MAX_ANGLE / 2 - angle) * (PI / 180)); // Rotation angle
    int xMid = w / 2;
    int yMid = h / 2;
    double cosA = cos(alpha);
    double sinA = sin(alpha);

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            Uint32 pixel = pixels[x * w + y];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
            if (r != 0) // if pixel not white
            {
                int newX = (x - xMid) * cosA - (y - yMid) * sinA + xMid;
                int newY = (x - xMid) * sinA + (y - yMid) * cosA + yMid;

                if (newX >= 0 && newX < w && newY >= 0 && newY < h) {
                    newPixels[newX * h + newY] = SDL_MapRGB(format, 255, 255, 255); // Put a black pixel
                }
            }
        }
    }
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
    return newSurface;
}


// function to create an image with a number on it
void createImageWithNumber(const char *filename, TTF_Font *font, int width,
                           int height, int number, long randomNumber) {

    // create a blank surface with the specified dimensions
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

    // check if the surface was created successfully
    if (surface == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurface Error: %s\n", SDL_GetError());
        return;
    }

    // fill the surface with a white background
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));


    // create a color for the text (black in this case)
    SDL_Color textColor = {255, 255, 255, 255};

    // create a text surface with the number
    char text[10]; // Int to char
    snprintf(text, sizeof(text), "%d", number);
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, textColor);

    // check if the text surface was created successfully
    if (textSurface == NULL) {
        fprintf(stderr, "TTF_RenderText_Blended Error: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        SDL_FreeSurface(surface);
        return;
    }


    // calculate the position to center the text on the surface (with a little random decal number)
    int textX = (width - textSurface->w) / 2 - DECAL_MAX / 2 + (randomNumber % DECAL_MAX);
    int textY = (height - textSurface->h) / 2 - DECAL_MAX / 2 + (randomNumber % DECAL_MAX);

    // blit the text surface onto the blank surface
    SDL_BlitSurface(textSurface, NULL, surface, &(SDL_Rect) {textX, textY, 0, 0});

    // pixels operations
    SDL_Surface *modifiedSurface = rotate(surface, (randomNumber % (MAX_ANGLE * 2))); // Rotate random with max angle

    // save the surface as an image file
    if (IMG_SavePNG(modifiedSurface, filename) != 0) {
        fprintf(stderr, "IMG_SavePNG Error: %s\n", IMG_GetError());
    }

    // free elements
    SDL_FreeSurface(modifiedSurface);
    SDL_FreeSurface(textSurface);
}

// main function that use others. Generate the dataset.
void generateDataSet(int nbIteration, char *folderPath) {

    char *fonts = readFontsName();
    char extension[] = ".png";

    // Create a window and renderer
    SDL_Window *window = SDL_CreateWindow("Image with Number", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 50,
                                          50, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    // Get fonts
    char *fontsDirectory = "fonts/";
    size_t directorySize = getStringLength(fontsDirectory, 0);
    size_t indexFonts = 0;
    mode_t mode = 0777;
    /*for (int j = 1; j <= 9; ++j) {
        char path[100];
        sprintf(path, "%s%i", folderPath, j);
        printf("%s\n", path);
        mkdir(path, mode);
    }*/
    for (int k = 0; k < nb_fonts; ++k) {

        // fonts
        size_t len_actual_font = getStringLength(fonts, indexFonts);
        char fontPath[directorySize + len_actual_font + 1];
        for (size_t l = 0; l < directorySize; ++l) {
            fontPath[l] = fontsDirectory[l];
        }

        for (size_t j = 0; j < len_actual_font; ++j) {
            fontPath[directorySize + j] = fonts[indexFonts + j];
        }
        fontPath[directorySize + len_actual_font] = '\0';

        indexFonts = indexFonts + len_actual_font + 1;

        printf("Create with font : %s \n", fontPath);

        long randomNumber = (rand() * ((k) % RAND_MAX)) % RAND_MAX;
        //printf("Random number: %ld\n", randomNumber);
        // Generate and print a random number between 0 and RAND_MAX
        // Load a font for rendering text
        int fontSize = MIN_FONT_SIZE + (randomNumber % (MAX_FONT_SIZE - MIN_FONT_SIZE));
        TTF_Font *font = TTF_OpenFont(fontPath, fontSize);
        if (font == NULL) {
            fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
            SDL_Quit();
            return;
        }
        for (int j = 1; j <= 9; ++j) {

            int i = nbIteration * k;
            while (i < nbIteration * (k + 1)) {
                char filePath[100];
                sprintf(filePath, "%s%d%s%05d%s", folderPath, j, "_", i, extension);


                // now, 'filePath' contains the desired file path
                // printf("File Path: %s\n", filePath);

                srand(time(NULL));

                createImageWithNumber(filePath, font, 28, 28, j, randomNumber);
                i++;
                randomNumber = (rand() % 500 * ((i) % 100)) % RAND_MAX;
                fontSize = MIN_FONT_SIZE + (randomNumber % (MAX_FONT_SIZE - MIN_FONT_SIZE));
                TTF_SetFontSize(font, fontSize);
                //  printf("Random number: %ld\n", randomNumber);
            }
        }
        TTF_CloseFont(font);

    }

    // cleanup resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    free(fonts);
}


int main() {

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Init TTF
    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    printf("It's here to create some images. First how many image of each number and each font do you want ?\n");
    printf("For example 10 create 10 * 9 * nb_fonts images\n");
    int nbIterations;
    printf("So ? How many ? : ");

    if (scanf("%d", &nbIterations) == 1) {

        printf("Create %i images of each fonts (and each digit) ...\n", nbIterations);
        generateDataSet(nbIterations, PATH);
        printf("%i images created.\n", nbIterations * 9 * nb_fonts);

        return EXIT_SUCCESS;
    } else {

        printf("L'entrée n'est pas un chiffre valide.\n");
        return EXIT_FAILURE;
    }

}


// to test the load of fonts
/*
char *fonts = readFontsName();
printFontTable(fonts, nb_fonts);
size_t len = 0;
len = getStringLength(fonts, len);
printf("Length fonts[0] = %zu\n", len);
len++;
len = getStringLength(fonts, len);
printf("Length fonts[1] = %zu\n", len);

size_t indexFonts = 0;
for (int i = 0; i < nb_fonts; ++i) {
    size_t len_actual_font = getStringLength(fonts, indexFonts);
    char font[len_actual_font + 1];
    for (size_t j = 0; j < len_actual_font; ++j) {
        font[j] = fonts[indexFonts + j];
    }
    font[len_actual_font] = '\0';

    indexFonts = indexFonts + len_actual_font + 1;

    printFontTable(font, 1);
}
free(fonts);*/
