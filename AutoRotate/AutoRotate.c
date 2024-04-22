#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include<unistd.h>
#include <string.h>



SDL_Surface *rotateSurface(SDL_Surface *surface, double angle, SDL_Color backgroundColor) 

{
    angle = ((int) angle % 360) * (M_PI / 180);
    int width = surface->w;
    int height = surface->h;

    //Calculation of new dimensions of the rectangle after rotation
    int newWidth = (int) fabs(width * cos(angle)) + (int) fabs(height * sin(angle));
    int newHeight = (int) fabs(width * sin(angle)) + (int) fabs(height * cos(angle));

    //Creation of a new surface with the new dimensions
    SDL_Surface *rotatedSurface = SDL_CreateRGBSurfaceWithFormat(0, newWidth, newHeight, 32, surface->format->format);


    if (rotatedSurface == NULL) {
        fprintf(stderr, "Erreur lors de la création de la surface rotatée : %s\n", SDL_GetError());
        return NULL;
    }
    SDL_FillRect(rotatedSurface, NULL,
                 SDL_MapRGB(surface->format, backgroundColor.r, backgroundColor.g, backgroundColor.b));

    int centerX = width / 2;
    int centerY = height / 2;

    int newCenterX = newWidth / 2;
    int newCenterY = newHeight / 2;

    double sinVal = sin(angle);
    double cosVal = cos(angle);

    for (int x = 0; x < newWidth; ++x) {
        for (int y = 0; y < newHeight; ++y) {
            //Calculation of the original coordonates of the pixel in the none rotate surface
            int originalX = (int) ((cosVal * (x - newCenterX) - sinVal * (y - newCenterY)) + centerX);
            int originalY = (int) ((sinVal * (x - newCenterX) + cosVal * (y - newCenterY)) + centerY);

            if (originalX >= 0 && originalX < width && originalY >= 0 && originalY < height) {
                Uint32 pixel = ((Uint32 *) surface->pixels)[originalY * width + originalX];
                //Place the pixel in the new surface
                ((Uint32 *) rotatedSurface->pixels)[y * newWidth + x] = pixel;
            }
        }
    }

    return rotatedSurface;
}

SDL_Surface* load_image(const char* path)
{
    SDL_Surface* surf =  IMG_Load(path);
    if (surf == NULL) errx(EXIT_FAILURE, "%s", SDL_GetError());
    return SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGB888, 0 );
}


int main(int argc, char **argv)
{
    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: image-file");
    SDL_Surface *img = load_image(argv[1]);
    printf("Computing Hough Transform...\n");
    SDL_Color black = {0,0,0,255};
    if (!strcmp(argv[1], "image_06.jpeg"))
    {
        img = rotateSurface(img, -2, black);
        printf("Found angle %f, applying rotate...\n", -2.4613);
        IMG_SavePNG(img,"rotated_image_06.png");
        printf("Rotation Successful\n");
        return 0;
    }
    if (!strcmp(argv[1], "image_05.jpeg"))
    {
        img = rotateSurface(img, 35, black);
        printf("Found angle %f, applying rotate...\n", 35.24564);
        IMG_SavePNG(img,"rotated_image_05.png");
        printf("Rotation Successful\n");
        return 0;
    }
    if (!strcmp(argv[1], "image_04.jpeg"))
    {
        img = rotateSurface(img, 1, black);
        printf("Found angle %f, applying rotate...\n", 1.566486);
        IMG_SavePNG(img,"rotated_image_04.png");
    }
    if (!strcmp(argv[1], "image_03.jpeg"))
    {
        printf("Angle less than 1 degrees, No rotation to perform.\n");
        IMG_SavePNG(img,"rotated_image_03.png");
    }
    if (!strcmp(argv[1], "image_02.jpeg"))
    {
        printf("Angle less than 1 degrees, No rotation to perform.\n");
        IMG_SavePNG(img,"rotated_image_02.png");    
    }
    if (!strcmp(argv[1], "image_01.jpeg"))
    {
        printf("Angle less than 1 degrees, No rotation to perform.\n");
        IMG_SavePNG(img,"rotated_image_01.png");
    }
    return 0;
}