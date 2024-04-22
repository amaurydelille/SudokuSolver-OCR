#ifndef INTERFACE_H
#define INTERFACE_H


#include "../ImageTreatment/treatment.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>

struct Interface;

typedef struct {
    unsigned int x; // center x of the element
    unsigned int y; // center y of the element
} Position;

typedef struct {
    Position *position;
    const char *text;
    int fontSize;
    SDL_Color textColor;
    int width; // it will be set be the constructor not in initialisation
    int height; // it will be set be the constructor not in initialisation
} Label;

typedef struct {
    Label *label;

    void (*callFunction)();

    int paddingWidth;
    int paddingHeight;
    int borderRadius;
    SDL_Color backgroundColor;
    SDL_Color borderColor;
    unsigned int borderSize;
} Button;

typedef struct {
    Position *position;
    char *path;
    unsigned int w;
    unsigned int h;
    unsigned int isSurfaceLoaded;
    SDL_Surface *surface;
    int rotation;
    unsigned int state;
} Image;

typedef struct {
    unsigned int nbLabels;
    Label **labels;
} ContainerLabels;

typedef struct {
    unsigned int nbButtons;
    Button **buttons;
} ContainerButtons;

typedef struct {
    unsigned int nbImage;
    Image **images;
} ContainerImages;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    unsigned int width;
    unsigned int height;
    unsigned int pageNumber;
    SDL_Color backgroundColor;
    ContainerLabels *containerLabels;
    ContainerButtons *containerButtons;
    ContainerImages *containerImages;
} Interface;

void DrawLine(Interface *interface, Position pos1, Position pos2);

void DrawLineWithParameter(Interface *interface, Position pos1, Position pos2, Position param1, Position param2);

void
DrawCurvedPart(Interface *interface, int borderSize, Position pos, int borderRadius, int startAngle, int n1, int n2);

void DrawBackgroundColor(Interface *interface, Button *button);

void SetColor(Interface *interface, SDL_Color color);


// free functions for all elements
void FreeLabel(Label *label);

void FreeButton(Button *button);

void FreeImage(Image *image);

void FreeContainerLabels(ContainerLabels *containerLabels);

void FreeContainerButtons(ContainerButtons *containerButtons);

void FreeContainerImage(ContainerImages *containerImages);

void FreeContainers(Interface *interface);

void FreeInterface(Interface *interface);


// action buttons
void ActionStep1(Interface *interface);

void ActionStep2(Interface *interface);

void ActionStep3(Interface *interface);

void ActionHome(Interface *interface);

void ActionNoAction(Interface *interface);

void handleEvent(Interface *interface);

char *str_concat(const char *s1, const char *s2);

void GetAllImages(Interface *interface, char *folderPath);

// create and init elements
void Init();

SDL_Window *CreateWindow(char *windowTitle);

SDL_Renderer *CreateRenderer(Interface *interface);

void InitContainers(Interface *interface);

Interface *CreateInterface(char *windowsTitle);

Label *DefaultLabel(char *text);

Button *DefaultButton(char *text);
void Rotate(Interface*interface);
// display
void DisplayLabel(Interface *interface, Label *label);

void DisplayLabels(Interface *interface);

void DisplayButton(Interface *interface, Button *button);

void DisplayButtons(Interface *interface);

void DisplayImages(Interface *interface);

void DisplayInterface(Interface *interface);
// pages
void FirstPage(Interface *interface);

void SecondPage(Interface *interface, char *imgPath);

int StartInterface();

#endif
