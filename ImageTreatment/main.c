#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <unistd.h>
#include "treatment.h"


int main(int argc, char *argv[]) {
    //resizeImageWrapper("test.png","test.png", 28, 28);

     if (argc < 5) {
        printf("Usage: %s <image or directory (0 image or 1 directory)> <input_image/directory> <output_image/directory> <debug (0 or 1)>\n",
               argv[0]);
        return 1;
    }
    if (atoi(argv[1])) {
        DIR *d;
        struct dirent *dir;
        d = opendir(argv[2]);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_name[0] != '.'){
                    char filePath[300];
                    sprintf(filePath, "%s/%s", argv[2], dir->d_name);
                    char outputPath[300];
                    sprintf(outputPath, "%s/%s", argv[3], dir->d_name);
                    applyTreatment(filePath, outputPath, atoi(argv[4]));
                }
            }
            closedir(d);
        }
        return EXIT_SUCCESS;
    } else {
        return applyTreatment(argv[2], argv[3], atoi(argv[4]));
    }
}
