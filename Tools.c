#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include<unistd.h>


SDL_Texture *resizeImage(SDL_Renderer *renderer, const char *path, int new_width, int new_height) {
    // Charger l'image originale
    SDL_Surface *originalSurface = IMG_Load(path);
    if (!originalSurface) {
        printf("Erreur lors du chargement de l'image : %s\n", IMG_GetError());
        return NULL;
    }

    // Convertir la surface en texture
    SDL_Texture *originalTexture = SDL_CreateTextureFromSurface(renderer, originalSurface);
    SDL_FreeSurface(originalSurface); // La surface n'est plus nécessaire
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

// Fonction d'appel pour redimensionner une image
void resizeImageWrapper(const char *imagePath, const char *outputPath, int newWidth, int newHeight) {
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

    // Appel de la fonction resizeImage
    SDL_Texture *resizedImage = resizeImage(renderer, imagePath, newWidth, newHeight);
    if (!resizedImage) {
        printf("Erreur lors du redimensionnement de l'image\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return;
    }
    saveTextureAsImage(renderer, resizedImage, outputPath, newWidth, newHeight);
    // Nettoyage et renvoi de la texture redimensionnée
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    SDL_DestroyTexture(resizedImage);
}
