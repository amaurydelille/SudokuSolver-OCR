#pragma once
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <time.h>

#define INPUTS 784
#define HIDDEN 10
#define OUTPUTS 10
#define LEARNING_RATE 0.1
#define EPOCHS 150
#define SAMPLES 45000
#define TOTAL_IMAGES 45000


typedef struct NeuralNetwork {
        double* inputs;
        double* hidden;
        double* output;
        double* bias_1;
        double* bias_2;
        double** weights_1;
        double** weights_2;
        double* dZ1;
        double* dZ2;
        double** dW1;
        double** dW2;
        double* db1;
        double* db2;
}NeuralNetwork;

double randn();
double random_weights();
double reLu(double x);
double relu_derivative(double x);
void softmax(NeuralNetwork* nn);
void initialize_neuralnetwork(NeuralNetwork* nn);
void forward_propagation(NeuralNetwork* nn);
void back_propagation(NeuralNetwork* nn, int* Y, int i);
void update_params(NeuralNetwork* nn);
int* get_predictions(double* A2, int outputs, int samples);
double get_accuracy(int* predictions, int* Y, int samples) ;
int getposmax(double* x);
void load_params(NeuralNetwork* nn);
void save_params(NeuralNetwork* nn);
int predict(NeuralNetwork* nn, double* image);
double getmax(double* x);
void freeNeuralNetwork(NeuralNetwork* nn);
