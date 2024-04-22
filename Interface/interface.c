#include "interface.h"

#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 800

#define FIRST_COLUMN_BUTTONS_X 150
#define SECOND_COLUMN_BUTTONS_X 400

#define PAGE2_IMG_DECAL 200

void DrawLine(Interface *interface, Position pos1, Position pos2) {
    SDL_RenderDrawLine(interface->renderer, pos1.x, pos1.y, pos2.x, pos2.y);
}

void DrawLineWithParameter(Interface *interface, Position pos1, Position pos2, Position param1, Position param2) {
    SDL_RenderDrawLine(interface->renderer, pos1.x + param1.x, pos1.y + param1.y, pos2.x + param2.x, pos2.y + param2.y);
}

void
DrawCurvedPart(Interface *interface, int borderSize, Position pos, int borderRadius, int startAngle, int n1, int n2) {
    for (int angle = startAngle; angle <= startAngle + 90; angle++) {
        double radians = angle * M_PI / 180.0;
        int x = pos.x + (borderRadius * n1) + (int) (borderRadius * cos(radians));
        int y = pos.y + (borderRadius * n2) + (int) (borderRadius * sin(radians));
        SDL_RenderDrawPoint(interface->renderer, x, y);
        for (int i = 1; i < borderSize; ++i) {
            SDL_RenderDrawPoint(interface->renderer, x + (i * n1), y);
            SDL_RenderDrawPoint(interface->renderer, x, y + (i));
        }
    }
}

void DrawBackgroundColor(Interface *interface, Button *button) {
    SDL_SetRenderDrawColor(interface->renderer, button->backgroundColor.r, button->backgroundColor.g,
                           button->backgroundColor.b,
                           button->backgroundColor.a);
    int startX = button->label->position->x - ((button->label->width + (button->paddingWidth * 2)) / 2);
    int startY = button->label->position->y - ((button->label->height + (button->paddingHeight * 2)) / 2);
    for (int x = 0; x < button->label->width + (2 * button->paddingWidth); ++x) {
        for (int y = 0; y < button->label->height + (2 * button->paddingHeight); ++y) {
            SDL_RenderDrawPoint(interface->renderer, x + startX, y + startY);
        }
    }
}

void SetColor(Interface *interface, SDL_Color color) {
    SDL_SetRenderDrawColor(interface->renderer, color.r, color.g, color.b, color.a);
}

void DrawRectangleButton(Interface *interface, Button *button) {
    SDL_SetRenderDrawColor(interface->renderer, button->borderColor.r, button->borderColor.g, button->borderColor.b,
                           button->borderColor.a);
    int xCenter = button->label->position->x; // center of the element
    int yCenter = button->label->position->y; // center of the element

    int halfWidth = (button->label->width + (button->paddingWidth * 2)) / 2;
    int halfHeight = (button->label->height + (button->paddingHeight * 2)) / 2;


    Position tl; // top left
    tl.x = xCenter - halfWidth;
    tl.y = yCenter - halfHeight;
    Position tr; // top right
    tr.x = xCenter + halfWidth;
    tr.y = yCenter - halfHeight;

    Position bl; // bottom left
    bl.x = xCenter - halfWidth;
    bl.y = yCenter + halfHeight;

    Position br; // bottom right
    br.x = xCenter + halfWidth;
    br.y = yCenter + halfHeight;

    // background
    // curve
    for (int angle = 180; angle < 270; ++angle) {
        double radian = angle * M_PI / 180;
        int xStart = tl.x + button->borderRadius + (int) (button->borderRadius * cos(angle * M_PI / 180));
        int xEnd = tr.x - button->borderRadius + (int) (button->borderRadius * cos((angle + 180) * M_PI / 180));
        int yTop = tl.y + button->borderRadius + (int) (button->borderRadius * sin(radian));

        int yBottom = bl.y - button->borderRadius + (int) (button->borderRadius * sin((angle + 180) * M_PI / 180));

        SetColor(interface, button->backgroundColor);
        SDL_RenderDrawLine(interface->renderer, xStart, yTop, xEnd, yTop);
        SDL_RenderDrawLine(interface->renderer, xStart, yBottom, xEnd, yBottom);
    }


    // middle background

    SetColor(interface, button->backgroundColor);
    for (unsigned int y = tl.y + button->borderRadius; y <= bl.y - button->borderRadius; ++y) {
        for (unsigned int x = tl.x + button->borderSize - 1; x <= tr.x - button->borderSize + 1; ++x) {
            SDL_RenderDrawPoint(interface->renderer, x, y);
        }
    }

    // border

    SetColor(interface, button->borderColor);
    // middle
    for (unsigned int i = 0; i < button->borderSize; ++i) {
        SDL_RenderDrawLine(interface->renderer, tl.x + i, tl.y + button->borderRadius, tl.x + i,
                           bl.y - button->borderRadius);

        SDL_RenderDrawLine(interface->renderer, tr.x - i, tl.y + button->borderRadius, tr.x - i,
                           bl.y - button->borderRadius);
    }
    // curve
    DrawCurvedPart(interface, button->borderSize, tl, button->borderRadius, 180, 1, 1);
    DrawCurvedPart(interface, button->borderSize, tr, button->borderRadius, 270, -1, 1);
    DrawCurvedPart(interface, button->borderSize, br, button->borderRadius, 0, -1, -1);
    DrawCurvedPart(interface, button->borderSize, bl, button->borderRadius, 90, 1, -1);

    for (unsigned int i = 0; i < button->borderSize; ++i) {
        SetColor(interface, button->borderColor);
        SDL_RenderDrawLine(interface->renderer, tl.x + button->borderRadius, tl.y + i, tr.x - button->borderRadius,
                           tl.y + i);
        SDL_RenderDrawLine(interface->renderer, bl.x + button->borderRadius, bl.y + i, br.x - button->borderRadius,
                           bl.y + i);
    }
}


void FreeLabel(Label *label) {
    free(label->position);
    free(label);
}

void FreeButton(Button *button) {
    FreeLabel(button->label);
    free(button);
}

void FreeImage(Image *image) {
    //free(image->position);
    free(image);
}

void FreeContainerLabels(ContainerLabels *containerLabels) {
    for (unsigned int i = 0; i < containerLabels->nbLabels; ++i) {
        FreeLabel(containerLabels->labels[i]);
    }
    if (containerLabels->nbLabels > 0)
        free(containerLabels->labels);
}

void FreeContainerButtons(ContainerButtons *containerButtons) {
    for (unsigned int i = 0; i < containerButtons->nbButtons; ++i) {
        FreeButton(containerButtons->buttons[i]);
    }
    if (containerButtons->nbButtons > 0)
        free(containerButtons->buttons);
}

void FreeContainerImage(ContainerImages *containerImages) {
    if (containerImages->nbImage == 1 && containerImages->images[0]->isSurfaceLoaded)
        SDL_FreeSurface(containerImages->images[0]->surface);
    for (unsigned int i = 0; i < containerImages->nbImage; ++i) {
        FreeImage(containerImages->images[i]);
    }
    if (containerImages->nbImage > 0)
        free(containerImages->images);
}

void FreeContainers(Interface *interface) {
    FreeContainerLabels(interface->containerLabels);
    FreeContainerButtons(interface->containerButtons);
    FreeContainerImage(interface->containerImages);
    free(interface->containerLabels);
    free(interface->containerButtons);
    free(interface->containerImages);
}

void FreeInterface(Interface *interface) {
    FreeContainers(interface);
    free(interface);
}

// action buttons

void ActionDelChange(Interface *interface) {
    SecondPage(interface, interface->containerImages->images[0]->path);
}

void ActionToGrayScale(Interface *interface) {
    SDL_Surface *img = SDL_ConvertSurfaceFormat(IMG_Load(interface->containerImages->images[0]->path),
                                                SDL_PIXELFORMAT_RGB888, 0);
    if (img == NULL)
        errx(EXIT_FAILURE, "ToGrayScale image load error");

    surface_to_grayscale(img);

    interface->containerImages->images[0]->isSurfaceLoaded = 1;
    interface->containerImages->images[0]->surface = img;
    interface->containerImages->images[0]->state = 2;
    Rotate(interface);
}

void ActionToBlackAndWhite(Interface *interface) {
    SDL_Surface *img = SDL_ConvertSurfaceFormat(IMG_Load(interface->containerImages->images[0]->path),
                                                SDL_PIXELFORMAT_RGB888, 0);
    if (img == NULL)
        errx(EXIT_FAILURE, "ToBlackAndWhite image load error");

    surface_to_blackWhite(img);

    interface->containerImages->images[0]->isSurfaceLoaded = 1;
    interface->containerImages->images[0]->surface = img;
    interface->containerImages->images[0]->state = 3;
    Rotate(interface);
}

void Rotate(Interface *interface) {
    SDL_Surface *img = SDL_ConvertSurfaceFormat(IMG_Load(interface->containerImages->images[0]->path),
                                                SDL_PIXELFORMAT_RGB888, 0);
    switch (interface->containerImages->images[0]->state) {
        case 1: // img without modification
            break;
        case 2: // grayscale
            surface_to_grayscale(img);
            break;
        case 3: // b&w
            surface_to_blackWhite(img);
            break;
        case 4: // b&w
            apply_gaussian_blur(img);
            break;
    }

    SDL_Surface *rotatedSurface = rotateSurface(img, interface->containerImages->images[0]->rotation,
                                                interface->backgroundColor);
    SDL_FreeSurface(img);
    SDL_FreeSurface(interface->containerImages->images[0]->surface);
    interface->containerImages->images[0]->surface = rotatedSurface;
    DisplayInterface(interface);
}

void ActionRotateLeft(Interface *interface) {
    interface->containerImages->images[0]->rotation -= 3;
    Rotate(interface);
}

void ActionRotateRight(Interface *interface) {
    interface->containerImages->images[0]->rotation += 3;
    Rotate(interface);
}

void ActionStep1(Interface *interface) {
    printf("Action step 1\n");
    interface->containerButtons->nbButtons += 2;
    interface->containerButtons->buttons = realloc(interface->containerButtons->buttons,
                                                   sizeof(Button *) * interface->containerButtons->nbButtons);
    Button *GrayScaleButton = DefaultButton("GrayScale");
    GrayScaleButton->label->position->x = SECOND_COLUMN_BUTTONS_X;
    GrayScaleButton->label->position->y = 100;
    GrayScaleButton->backgroundColor = (SDL_Color) {166, 166, 166, 255};
    GrayScaleButton->callFunction = ActionToGrayScale;
    Button *BlackAndWhiteButton = DefaultButton("Black & White");
    BlackAndWhiteButton->label->position->x = SECOND_COLUMN_BUTTONS_X;
    BlackAndWhiteButton->label->position->y = 170;
    BlackAndWhiteButton->backgroundColor = (SDL_Color) {255, 255, 255, 255};
    BlackAndWhiteButton->callFunction = ActionToBlackAndWhite;
    interface->containerButtons->buttons[interface->containerButtons->nbButtons - 2] = GrayScaleButton;
    interface->containerButtons->buttons[interface->containerButtons->nbButtons - 1] = BlackAndWhiteButton;

    DisplayInterface(interface);
}

void ActionStep2(Interface *interface) {
    printf("Action step 2\n");
    SDL_Surface *img = SDL_ConvertSurfaceFormat(IMG_Load(interface->containerImages->images[0]->path),
                                                SDL_PIXELFORMAT_RGB888, 0);
    if (img == NULL)
        errx(EXIT_FAILURE, "ToBlackAndWhite image load error");

    apply_gaussian_blur(img);

    interface->containerImages->images[0]->isSurfaceLoaded = 1;
    interface->containerImages->images[0]->surface = img;
    interface->containerImages->images[0]->state = 4;


    /*printf("Action step 2\n");
     interface->containerLabels->nbLabels +=1;
     Label * newLabel = DefaultLabel("This action is not implemented yet.");
     newLabel->position->x = SECOND_COLUMN_BUTTONS_X;
     newLabel->position->y = WINDOW_HEIGHT-200;
     interface->containerLabels->labels[interface->containerLabels->nbLabels-1] =newLabel;
     DisplayInterface(interface);*/
}

void ActionStep3(Interface *interface) {
    printf("Action step 3\n");
    interface->containerLabels->nbLabels += 1;
    Label *newLabel = DefaultLabel("This action is not implemented yet.");
    newLabel->position->x = SECOND_COLUMN_BUTTONS_X;
    newLabel->position->y = WINDOW_HEIGHT - 200;
    interface->containerLabels->labels[interface->containerLabels->nbLabels - 1] = newLabel;
    DisplayInterface(interface);
}

void ActionHome(Interface *interface) {
    printf("Action go home\n");
    FirstPage(interface);
    printf("First page\n");

}

void ActionNoAction(Interface *interface) {
    printf("No action selected\n");
    interface->containerLabels->nbLabels += 1;
    Label *newLabel = DefaultLabel("This action is not implemented yet.");
    newLabel->position->x = SECOND_COLUMN_BUTTONS_X;
    newLabel->position->y = WINDOW_HEIGHT - 200;
    interface->containerLabels->labels[interface->containerLabels->nbLabels - 1] = newLabel;
    DisplayInterface(interface);
}

void handleEvent(Interface *interface) {
    SDL_Event event;
    int mouse_x;
    int mouse_y;
    while (1) {
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT: // quit window -> exit the program
                return;
            case SDL_MOUSEBUTTONDOWN: // mouse click
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                //printf("Mouse click : x=%i / y=%i \n", mouse_x, mouse_y);
                // printf("page nb%u\n", interface->pageNumber);
                if (interface->pageNumber == 1) { // first page
                    for (unsigned int i = 0; i < interface->containerImages->nbImage; ++i) { // if click on a image
                        int imgW = interface->containerImages->images[i]->w;
                        int imgH = interface->containerImages->images[i]->h;
                        int imgPosX = interface->containerImages->images[i]->position->x;
                        int imgPosY = interface->containerImages->images[i]->position->y;

                        if (mouse_x >= imgPosX - (imgW / 2) && mouse_x <= imgPosX + (imgW / 2) &&
                            mouse_y >= imgPosY - (imgH / 2) && mouse_y <= imgPosY + (imgH / 2)) {
                            printf("Selected image :%s\n", interface->containerImages->images[i]->path);
                            SecondPage(interface, interface->containerImages->images[i]->path);
                            break;
                        }
                    }
                } else if (interface->pageNumber == 2) {
                    for (unsigned int i = 0; i < interface->containerButtons->nbButtons; ++i) {
                        int imgW = interface->containerButtons->buttons[i]->label->width;
                        int imgH = interface->containerButtons->buttons[i]->label->height;
                        int imgPosX = interface->containerButtons->buttons[i]->label->position->x;
                        int imgPosY = interface->containerButtons->buttons[i]->label->position->y;

                        if (mouse_x >= imgPosX - (imgW / 2) && mouse_x <= imgPosX + (imgW / 2) &&
                            mouse_y >= imgPosY - (imgH / 2) && mouse_y <= imgPosY + (imgH / 2)) {
                            // printf("Selected button :%s\n", interface->containerButtons->buttons[i]->label->text);
                            interface->containerButtons->buttons[i]->callFunction(interface);
                            break;
                        }
                    }
                }
                break;
        }
    }
}

char *str_concat(const char *s1, const char *s2) {
    size_t len1 = 0;
    while (s1[len1] != '\0')
        len1++;
    size_t len2 = 0;
    while (s2[len2] != '\0')
        len2++;
    char *res = malloc(sizeof(char) * (len1 + len2 + 1));
    size_t i = 0;
    while (i < len1) {
        res[i] = s1[i];
        i++;
    }
    i = 0;
    while (i < len2) {
        res[i + len1] = s2[i];
        i++;
    }
    res[len1 + len2] = '\0';
    return res;
}

void GetAllImages(Interface *interface, char *folderPath) {
    unsigned int nbImage = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(folderPath);
    if (d) {
        while ((dir = readdir(d)) != NULL) { // count nb image
            if (dir->d_name[0] != '.') {
                nbImage++;
            }
        }
        closedir(d);
    } else {
        errx(EXIT_FAILURE, "Can not open images folder\n");
    }
    ContainerImages *containerImages = interface->containerImages;
    containerImages->nbImage = nbImage;
    containerImages->images = malloc(nbImage * sizeof(Image *));
    d = opendir(folderPath);
    size_t i = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] != '.') {
                Image *image = malloc(sizeof(Image));
                image->path = str_concat(folderPath, dir->d_name);
                containerImages->images[i] = image;
                i++;
            }
        }
        closedir(d);
    } else {
        errx(EXIT_FAILURE, "Can not open images folder\n");
    }
}

// create and init elements
void Init() {
    // Initializes the SDL.
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    // Init TTF
    if (TTF_Init() != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

}

SDL_Window *CreateWindow(char *windowTitle) {
    SDL_Window *window = SDL_CreateWindow(windowTitle, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    return window;
}

SDL_Renderer *CreateRenderer(Interface *interface) {
    SDL_Renderer *renderer = SDL_CreateRenderer(interface->window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        errx(EXIT_FAILURE, "%s", SDL_GetError());


    return renderer;
}

void InitContainers(Interface *interface) {
    interface->containerLabels = malloc(sizeof(ContainerLabels));
    interface->containerLabels->nbLabels = 0;
    interface->containerButtons = malloc(sizeof(ContainerButtons));
    interface->containerButtons->nbButtons = 0;
    interface->containerImages = malloc(sizeof(ContainerImages));
    interface->containerImages->nbImage = 0;

}

Interface *CreateInterface(char *windowsTitle) {
    Interface *interface = malloc(sizeof(Interface));
    interface->pageNumber = 1;
    interface->width = WINDOW_WIDTH;
    interface->height = WINDOW_HEIGHT;
    interface->window = CreateWindow(windowsTitle);
    interface->renderer = CreateRenderer(interface);
    interface->backgroundColor = (SDL_Color) {150, 150, 150, 255};
    InitContainers(interface);
    return interface;
}

Label *DefaultLabel(char *text) {
    Label *label = malloc(sizeof(Label));
    label->text = text;
    label->position = malloc(sizeof(Position));
    label->position->x = WINDOW_WIDTH / 2;
    label->position->y = 30;
    label->textColor = (SDL_Color) {0, 0, 0, 255};
    label->fontSize = 25;
    return label;
}

Button *DefaultButton(char *text) {
    Button *button = malloc(sizeof(Button));
    Label *label = DefaultLabel(text);
    button->label = label;
    button->paddingWidth = 20;
    button->paddingHeight = 10;
    button->backgroundColor = (SDL_Color) {245, 211, 211, 255};
    button->borderRadius = 10;
    button->borderColor = (SDL_Color) {0, 0, 0, 255};
    button->borderSize = 3;
    button->callFunction = ActionNoAction;
    return button;
}


// display
void DisplayLabel(Interface *interface, Label *label) {
    TTF_Font *font = TTF_OpenFont("arial.ttf", label->fontSize);
    if (font == NULL) {
        font = TTF_OpenFont("Interface/arial.ttf", label->fontSize);
        if (font == NULL)
            errx(EXIT_FAILURE, "%s", SDL_GetError());
    }

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, label->text, label->textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(interface->renderer, textSurface);
    label->width = textSurface->w;
    label->height = textSurface->h;

    SDL_Rect textRect;
    textRect.x = label->position->x - (textSurface->w / 2);
    textRect.y = label->position->y - (textSurface->h / 2);
    textRect.w = label->width;
    textRect.h = label->height;

    SDL_RenderCopy(interface->renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
}

void DisplayLabels(Interface *interface) {
    for (unsigned int i = 0; i < interface->containerLabels->nbLabels; ++i) {
        Label *label = interface->containerLabels->labels[i];
        DisplayLabel(interface, label);
    }

}

void DisplayButton(Interface *interface, Button *button) {
    DisplayLabel(interface, button->label);

    DrawRectangleButton(interface, button);

    DisplayLabel(interface, button->label);

}

void DisplayButtons(Interface *interface) {
    for (unsigned int i = 0; i < interface->containerButtons->nbButtons; ++i) {
        Button *button = interface->containerButtons->buttons[i];
        DisplayButton(interface, button);
    }
}

void DisplayImages(Interface *interface) {
    if (interface->containerImages->nbImage > 1) {

        unsigned int nbImagesByColumn = (unsigned int) (interface->containerImages->nbImage + 1) / 2;
        for (unsigned int i = 0; i < interface->containerImages->nbImage; ++i) {
            interface->containerImages->images[i]->position = malloc(sizeof(Position));
            interface->containerImages->images[i]->position->x =
                    ((i + 1) / ((double) nbImagesByColumn + 1)) * WINDOW_WIDTH;
            interface->containerImages->images[i]->position->y = (1 / (double) 3) * WINDOW_HEIGHT;
            if (i >= nbImagesByColumn) {
                interface->containerImages->images[i]->position->x =
                        ((i + 1 - nbImagesByColumn) / ((double) nbImagesByColumn + 1)) * WINDOW_WIDTH;
                interface->containerImages->images[i]->position->y = (2 / (double) 3) * WINDOW_HEIGHT;
            }
        }
        SetColor(interface, (SDL_Color) {0, 0, 0, 255});


        // load image
        for (unsigned int i = 0; i < interface->containerImages->nbImage; ++i) {
            /*  printf("containerImages->images[%i].pos : x = %i , y = %i\n", i,
                     interface->containerImages->images[i]->position->x,
                     interface->containerImages->images[i]->position->y);
              printf("%s\n", interface->containerImages->images[i]->path);*/
            SDL_Surface *surface = IMG_Load(interface->containerImages->images[i]->path);
            if (surface == NULL)
                errx(EXIT_FAILURE, "%s", SDL_GetError());

            int newW = WINDOW_WIDTH / (nbImagesByColumn + 2);
            int newH = (int) ((float) surface->h * (newW / (float) surface->w));;

            interface->containerImages->images[i]->w = newW;
            interface->containerImages->images[i]->h = newH;

            SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(interface->renderer, surface);
            if (imageTexture == NULL)
                errx(EXIT_FAILURE, "%s", SDL_GetError());

            SDL_Rect destinationRect;
            destinationRect.x = interface->containerImages->images[i]->position->x - (newW / 2);
            destinationRect.y = interface->containerImages->images[i]->position->y - (newH / 2);
            destinationRect.w = newW;
            destinationRect.h = newH;

            SDL_RenderCopy(interface->renderer, imageTexture, NULL, &destinationRect);

            SDL_DestroyTexture(imageTexture);
            SDL_FreeSurface(surface);
        }
    } else {
        if (interface->containerImages->nbImage == 1) {
            if (interface->containerImages->images[0]->isSurfaceLoaded == 1) {
                int newH = 600;
                int newW = (int) ((float) interface->containerImages->images[0]->surface->w *
                                  (newH / (float) interface->containerImages->images[0]->surface->h));

                SDL_Rect destinationRect;
                destinationRect.x = (WINDOW_WIDTH / 2) - (newW / 2) + PAGE2_IMG_DECAL;
                destinationRect.y = (WINDOW_HEIGHT / 2) - (newH / 2);
                destinationRect.w = newW;
                destinationRect.h = newH;
                SDL_RenderCopy(interface->renderer,
                               SDL_CreateTextureFromSurface(interface->renderer,
                                                            interface->containerImages->images[0]->surface),
                               NULL,
                               &destinationRect);
            } else {
                SDL_Surface *surface = IMG_Load(interface->containerImages->images[0]->path);
                if (surface == NULL)
                    errx(EXIT_FAILURE, "%s", SDL_GetError());

                int newH = 600;
                int newW = (int) ((float) surface->w * (newH / (float) surface->h));

                SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(interface->renderer, surface);
                if (imageTexture == NULL)
                    errx(EXIT_FAILURE, "%s", SDL_GetError());

                SDL_Rect destinationRect;
                destinationRect.x = (WINDOW_WIDTH / 2) - (newW / 2) + PAGE2_IMG_DECAL;
                destinationRect.y = (WINDOW_HEIGHT / 2) - (newH / 2);
                destinationRect.w = newW;
                destinationRect.h = newH;

                interface->containerImages->images[0]->w = newW;
                interface->containerImages->images[0]->h = newH;
                interface->containerImages->images[0]->position = malloc(sizeof(Position));
                interface->containerImages->images[0]->position->x = destinationRect.x;
                interface->containerImages->images[0]->position->y = destinationRect.y;

                SDL_RenderCopy(interface->renderer, imageTexture, NULL, &destinationRect);

                SDL_DestroyTexture(imageTexture);
                SDL_FreeSurface(surface);
            }
        }
    }
}

void DisplayInterface(Interface *interface) {

    SDL_RenderClear(interface->renderer);
    SDL_SetRenderDrawColor(interface->renderer, interface->backgroundColor.r, interface->backgroundColor.g,
                           interface->backgroundColor.b, interface->backgroundColor.a);
    SDL_RenderClear(interface->renderer);

    DisplayImages(interface);

    DisplayLabels(interface);

    DisplayButtons(interface);

    SDL_RenderPresent(interface->renderer);
}


// Pages
void FirstPage(Interface *interface) {
    FreeContainers(interface);
    InitContainers(interface);
    interface->pageNumber = 1;
    GetAllImages(interface, "../img/");


    Label *firstLabel = DefaultLabel("Choose an image to solve");

    interface->containerLabels->nbLabels = 1;
    interface->containerLabels->labels = malloc(sizeof(Label) * interface->containerLabels->nbLabels);
    interface->containerLabels->labels[0] = firstLabel;

    DisplayInterface(interface);
}

void SecondPage(Interface *interface, char *imgPath) {
    FreeContainers(interface);
    InitContainers(interface);
    interface->pageNumber = 2;


    Button *firstButton = DefaultButton("First treatment");
    firstButton->label->position->x = FIRST_COLUMN_BUTTONS_X;
    firstButton->label->position->y = 100;
    firstButton->callFunction = ActionStep1;
    firstButton->backgroundColor = (SDL_Color) {245, 211, 211, 255};

    Button *secondButton = DefaultButton("Grid detection");
    secondButton->label->position->x = FIRST_COLUMN_BUTTONS_X;
    secondButton->label->position->y = 170;
    secondButton->callFunction = ActionStep2;
    secondButton->backgroundColor = (SDL_Color) {230, 245, 211, 255};


    Button *thirdButton = DefaultButton("Solve");
    thirdButton->label->position->x = FIRST_COLUMN_BUTTONS_X;
    thirdButton->label->position->y = 240;
    thirdButton->callFunction = ActionStep3;
    thirdButton->backgroundColor = (SDL_Color) {211, 214, 245, 255};

    Button *homeButton = DefaultButton("Back home");
    homeButton->label->position->x = FIRST_COLUMN_BUTTONS_X;
    homeButton->label->position->y = WINDOW_HEIGHT - 50;
    homeButton->callFunction = ActionHome;
    homeButton->backgroundColor = (SDL_Color) {255, 50, 50, 255};


    Button *rotateLeft = DefaultButton("<-");
    rotateLeft->label->position->x = WINDOW_WIDTH / 2 - 50 + PAGE2_IMG_DECAL;
    rotateLeft->label->position->y = WINDOW_HEIGHT - 50;
    rotateLeft->backgroundColor = (SDL_Color) {50, 150, 50, 255};
    rotateLeft->callFunction = ActionRotateLeft;
    Button *rotateRight = DefaultButton("->");
    rotateRight->label->position->x = WINDOW_WIDTH / 2 + 50 + PAGE2_IMG_DECAL;
    rotateRight->label->position->y = WINDOW_HEIGHT - 50;
    rotateRight->backgroundColor = (SDL_Color) {50, 150, 50, 255};
    rotateRight->callFunction = ActionRotateRight;

    interface->containerButtons->nbButtons = 7;
    interface->containerButtons->buttons = malloc(sizeof(Button *) * interface->containerButtons->nbButtons);
    interface->containerButtons->buttons[0] = firstButton;
    interface->containerButtons->buttons[1] = secondButton;
    interface->containerButtons->buttons[2] = thirdButton;
    interface->containerButtons->buttons[3] = homeButton;
    interface->containerButtons->buttons[4] = rotateLeft;
    interface->containerButtons->buttons[5] = rotateRight;

    interface->containerImages->nbImage = 1;
    interface->containerImages->images = malloc(sizeof(Image *) * interface->containerImages->nbImage);
    Image *img = malloc(sizeof(Image));
    img->isSurfaceLoaded = 1;
    img->path = imgPath;
    img->rotation = 0;
    img->state = 1;
    interface->containerImages->images[0] = img;
    SDL_Surface *imgSurface = SDL_ConvertSurfaceFormat(IMG_Load(interface->containerImages->images[0]->path),
                                                       SDL_PIXELFORMAT_RGB888, 0);
    if (imgSurface == NULL)
        errx(EXIT_FAILURE, "ToGrayScale image load error");
    interface->containerImages->images[0]->surface = imgSurface;


    Label *label = DefaultLabel("Solve an image");
    interface->containerLabels->nbLabels = 1;
    interface->containerLabels->labels = malloc(sizeof(Label *) * interface->containerLabels->nbLabels);
    interface->containerLabels->labels[0] = label;


    Button *NoChange = DefaultButton("X");
    NoChange->label->position->x = WINDOW_WIDTH - 50;
    NoChange->label->position->y = 50;
    NoChange->backgroundColor = (SDL_Color) {100, 50, 50, 255};
    NoChange->callFunction = ActionDelChange;
    interface->containerButtons->buttons[6] = NoChange;


    DisplayInterface(interface);
}

int StartInterface() {
    Init();
    Interface *interface = CreateInterface("Sudoku solver");

    FirstPage(interface);

    handleEvent(interface);

    FreeInterface(interface);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}