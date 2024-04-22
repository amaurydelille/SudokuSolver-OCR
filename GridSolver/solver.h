#pragma once

#include <stdio.h>
//#include "../DigitRecognition/neuralnetwork.h"
//#include "../DigitRecognition/files.h"

#define GRID_SIZE 9

void newgrid(int grid[GRID_SIZE][GRID_SIZE]);

void displaygrid(int grid[GRID_SIZE][GRID_SIZE]);

int randint(int min, int max);

int isvalid(int grid[GRID_SIZE][GRID_SIZE], int row, int column, int guess);

int solve(int grid[GRID_SIZE][GRID_SIZE]);

int findemptycell(int grid[][9], int *row, int *column);

void randomfill(int level, int grid[GRID_SIZE][GRID_SIZE]);

void writegridtofile(int grid[GRID_SIZE][GRID_SIZE], FILE *file);

void solveWithNeuralNetwork();