#include "solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include "../DigitRecognition/neuralnetwork.h"
#include "../DigitRecognition/files.h"

//Cette fonction va chercher dans le dossier /images et charge toutes les images qui y sont pour reconstruire la grille.
//Dans image il faut stocker sous la forme : 1.png, 2.png, etc.. 
//Le resultat de main est grid.result
void images_to_file() {
    FILE *read;
    read = fopen("squares/", "r");
    FILE *write;
    write = fopen("sources/grid", "w");
    NeuralNetwork *nn = malloc(sizeof(NeuralNetwork));
    initialize_neuralnetwork(nn);
    load_params(nn);

    char image_path[256];

    for (size_t i = 1; i <= 81; i++) {
        sprintf(image_path, "squares/%zu.png", i);
        double *image = path_to_input(image_path);
        int digit = predict(nn, image);
        fprintf(write, "%c", digit > 0 ? '0' + digit : '.');

        if ((i) % 3 == 0 && (i) % 9 != 0) {
            fprintf(write, " ");
        }

        if ((i) % 9 == 0) {
            fprintf(write, "\n");
            if ((i) % 27 == 0 && i != 81) {
                fprintf(write, "\n");
            }
        }
        fflush(write);
    }

    fclose(write);
    freeNeuralNetwork(nn);
    free(nn);
}


void filetogrid(int grid[GRID_SIZE][GRID_SIZE], char path[]) {
    FILE *file;
    file = fopen(path, "r");

    if (file == NULL)
        errx(1, "The file is not valid.");

    for (size_t i = 0; i < GRID_SIZE; i++) {
        for (size_t j = 0; j < GRID_SIZE; j++) {
            char number = fgetc(file);

            while (number == ' ' || number == '\n')
                number = fgetc(file);

            if (number == EOF) {
                errx(1, "The file contains too few characters.");
                fclose(file);
            }

            switch (number) {
                case '.':
                    grid[i][j] = 0;
                    break;

                case '1':
                    grid[i][j] = 1;
                    break;

                case '2':
                    grid[i][j] = 2;
                    break;

                case '3':
                    grid[i][j] = 3;
                    break;

                case '4':
                    grid[i][j] = 4;
                    break;

                case '5':
                    grid[i][j] = 5;
                    break;

                case '6':
                    grid[i][j] = 6;
                    break;

                case '7':
                    grid[i][j] = 7;
                    break;

                case '8':
                    grid[i][j] = 8;
                    break;

                case '9':
                    grid[i][j] = 9;
                    break;
            }
        }
    }

    fclose(file);
}

//Create a new board filled with '0'.
void newgrid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;
        }
    }
}

//Display the board in the standard output console.
void displaygrid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if ((j + 1) % 3 == 0)
                if (grid[i][j] == 0)
                    printf("%c ", '.');
                else
                    printf("%d ", grid[i][j]);
            else if (grid[i][j] == 0)
                printf("%c", '.');
            else
                printf("%d", grid[i][j]);
        }
        if ((i + 1) % 3 == 0)
            printf("\n\n");
        else
            printf("\n");
    }
}

void addnumber(int number, int x, int y, int grid[GRID_SIZE][GRID_SIZE]) {
    grid[x][y] = number;
}

//As the name suggest it, it returns a pseudo-random number in [min; max].
int randint(int min, int max) {
    static unsigned int seed = 12345;

    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;

    int range = max - min + 1;
    int random_number = (seed % range) + min;

    return random_number;
}

//This method find the first empty cell.
int findemptycell(int grid[][9], int *row, int *column) {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (!grid[i][j]) {
                *row = i;
                *column = j;

                return 1;
            }

    return 0;
}

//If a cell is legal we return 1, in the other case, 0.
int isvalid(int grid[GRID_SIZE][GRID_SIZE], int row, int column, int number) {
    int topX = row / 3 * 3;
    int topY = column / 3 * 3;

    for (int i = 0; i < 9; i++) {
        if (grid[row][i] == number)
            return 0;
        if (grid[i][column] == number)
            return 0;
        if (grid[topX + (i % 3)][topY + (i / 3)] == number)
            return 0;
    }
    return 1;
}

//Fill randomly the board with 3 levels, equivalent to 85%, 70%, 50%.
void randomfill(int level, int grid[GRID_SIZE][GRID_SIZE]) {
    int count;

    switch (level) {
        case 1:
            count = randint(70, 80);
            break;
        case 2:
            count = randint(35, 70);
            break;
        case 3:
            count = randint(21, 60);
            break;
    }

    int i = 0;
    do {
        int c = randint(1, 9);
        int x = randint(0, 8);
        int y = randint(0, 8);

        if (isvalid(grid, x, y, c)) {
            addnumber(c, x, y, grid);
            i++;
        }
    } while (i < count);
}

//This final function solve the final grid.
int solve(int grid[GRID_SIZE][GRID_SIZE]) {
    int row;
    int column;

    if (!findemptycell(grid, &row, &column))
        return 1;

    for (int number = 1; number < 10; number++)
        if (isvalid(grid, row, column, number)) {
            grid[row][column] = number;

            if (solve(grid))
                return 1;

            grid[row][column] = 0;
        }
    return 0;
}

void writegridtofile(int grid[GRID_SIZE][GRID_SIZE], FILE *file) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0)
                fputc('.', file);
            else
                fprintf(file, "%d", grid[i][j]);

            if ((j + 1) % 3 == 0 && j < GRID_SIZE - 1)
                fputc(' ', file);
        }
        fputc('\n', file);
        if ((i + 1) % 3 == 0 && i < GRID_SIZE - 1)
            fputs("\n", file);
    }
}

void solveWithNeuralNetwork() {
    images_to_file();

    char resultPath[256] = "sources/grid.result";

    FILE *result;
    result = fopen(resultPath, "w");

    if (result == NULL)
        errx(1, "Error while creating the file.");

    int grid[GRID_SIZE][GRID_SIZE];
    filetogrid(grid, "sources/grid");
    if (solve(grid)) {
        writegridtofile(grid, result);
    } else {
        printf("No solution was found.");
        fprintf(result, "No solution was found.");
    }

    fclose(result);
}
