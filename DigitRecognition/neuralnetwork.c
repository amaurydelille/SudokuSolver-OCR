#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "neuralnetwork.h"
#include "files.h"

double randn() {
    double u1, u2, w, mult;
    static double x1, x2;
    static int call = 0;

    if (call == 1) {
        call = !call;
        return x2;
    }

    do {
        u1 = -1 + ((double) rand() / RAND_MAX) * 2;
        u2 = -1 + ((double) rand() / RAND_MAX) * 2;
        w = pow(u1, 2) + pow(u2, 2);
    } while (w >= 1 || w == 0);

    mult = sqrt((-2 * log(w)) / w);
    x1 = u1 * mult;
    x2 = u2 * mult;

    call = !call;

    return x1;
}

double random_weights() {
    double r = randn() * sqrt(2.0 / 784);
    return r;
}

double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) {
    return x * (1.0 - x);
}

double reLu(double x) {
    if (x < 0)
        return 0;
    else
        return x;
}

double relu_derivative(double x) {
    return x > 0 ? 1 : 0;
}

void softmax(NeuralNetwork *nn) {
    double max_val = nn->output[0];
    for (size_t i = 1; i < OUTPUTS; i++) {
        if (nn->output[i] > max_val) {
            max_val = nn->output[i];
        }
    }

    double sum = 0.0;
    for (size_t i = 0; i < OUTPUTS; i++) {
        nn->output[i] = exp(nn->output[i] - max_val);
        sum += nn->output[i];
    }

    for (size_t i = 0; i < OUTPUTS; i++) {
        nn->output[i] /= sum;
    }
}

void initialize_neuralnetwork(NeuralNetwork *nn) {
    nn->inputs = malloc(sizeof(double) * INPUTS);
    nn->hidden = malloc(sizeof(double) * HIDDEN);
    nn->output = malloc(sizeof(double) * OUTPUTS);
    nn->bias_1 = malloc(sizeof(double) * HIDDEN);
    nn->bias_2 = malloc(sizeof(double) * OUTPUTS);
    nn->weights_1 = malloc(sizeof(double *) * HIDDEN);
    nn->weights_2 = malloc(sizeof(double *) * OUTPUTS);

    nn->dZ2 = malloc(sizeof(double) * OUTPUTS);
    nn->dZ1 = malloc(sizeof(double) * HIDDEN);
    nn->db1 = malloc(sizeof(double) * HIDDEN);
    nn->db2 = malloc(sizeof(double) * OUTPUTS);
    nn->dW2 = malloc(sizeof(double) * OUTPUTS);
    nn->dW1 = malloc(sizeof(double) * HIDDEN);

    for (size_t i = 0; i < HIDDEN; i++) {
        nn->weights_1[i] = malloc(sizeof(double) * INPUTS);
        nn->dW1[i] = malloc(sizeof(double) * INPUTS);
        nn->bias_1[i] = random_weights();
        nn->hidden[i] = random_weights();

        for (size_t j = 0; j < INPUTS; j++)
            nn->weights_1[i][j] = random_weights();
    }

    for (size_t i = 0; i < OUTPUTS; i++) {
        nn->weights_2[i] = malloc(sizeof(double) * HIDDEN);
        nn->dW2[i] = malloc(sizeof(double) * HIDDEN);
        nn->bias_2[i] = random_weights();
        nn->output[i] = random_weights();

        for (size_t j = 0; j < HIDDEN; j++)
            nn->weights_2[i][j] = random_weights();
    }
}

void freeNeuralNetwork(NeuralNetwork *nn) {
    free(nn->inputs);
    free(nn->hidden);
    free(nn->output);
    free(nn->bias_1);
    free(nn->bias_2);
    free(nn->dZ1);
    free(nn->dZ2);
    free(nn->db1);
    free(nn->db2);

    for (size_t i = 0; i < HIDDEN; i++) {
        free(nn->weights_1[i]);
        free(nn->dW1[i]);
    }

    for (size_t i = 0; i < OUTPUTS; i++) {
        free(nn->weights_2[i]);
        free(nn->dW2[i]);
    }

    free(nn->weights_1);
    free(nn->weights_2);
}

void forward_propagation(NeuralNetwork *nn) {
    for (size_t i = 0; i < HIDDEN; i++) {
        nn->hidden[i] = 0;
        for (size_t j = 0; j < INPUTS; j++)
            nn->hidden[i] += nn->inputs[j] * nn->weights_1[i][j];

        nn->hidden[i] = reLu(nn->hidden[i] + nn->bias_1[i]);
    }

    for (size_t i = 0; i < OUTPUTS; i++) {
        nn->output[i] = 0;
        for (size_t j = 0; j < HIDDEN; j++)
            nn->output[i] += nn->hidden[j] * nn->weights_2[i][j];

        nn->output[i] = nn->output[i] + nn->bias_2[i];
    }

    softmax(nn);
}

void back_propagation(NeuralNetwork *nn, int *Y, int i) {

    int one_hot_Y[OUTPUTS] = {0};
    one_hot_Y[Y[i]] = 1;

    for (size_t i = 0; i < OUTPUTS; i++)
        nn->dZ2[i] = nn->output[i] - one_hot_Y[i];

    for (size_t i = 0; i < OUTPUTS; i++) {
        for (size_t j = 0; j < HIDDEN; j++) {
            nn->dW2[i][j] = nn->dZ2[i] * nn->hidden[j] * 1 / SAMPLES;
        }
        nn->db2[i] = nn->dZ2[i] * 1 / SAMPLES;
    }

    for (size_t i = 0; i < HIDDEN; i++) {
        nn->dZ1[i] = 0;
        for (size_t j = 0; j < OUTPUTS; j++)
            nn->dZ1[i] += nn->weights_2[j][i] * nn->dZ2[j];
        nn->dZ1[i] *= relu_derivative(nn->hidden[i]);
    }

    for (size_t i = 0; i < HIDDEN; i++) {
        for (size_t j = 0; j < INPUTS; j++) {
            nn->dW1[i][j] = nn->dZ1[i] * nn->inputs[j];
        }
        nn->db1[i] = nn->dZ1[i];
    }
}

void update_params(NeuralNetwork *nn) {
    for (size_t i = 0; i < HIDDEN; i++) {
        for (size_t j = 0; j < INPUTS; j++)
            nn->weights_1[i][j] -= LEARNING_RATE * nn->dW1[i][j];
        nn->bias_1[i] -= LEARNING_RATE * nn->db1[i];
    }

    for (size_t i = 0; i < OUTPUTS; i++) {
        for (size_t j = 0; j < HIDDEN; j++)
            nn->weights_2[i][j] -= LEARNING_RATE * nn->dW2[i][j];
        nn->bias_2[i] -= LEARNING_RATE * nn->db2[i];
    }
}

int getposmax(double *x) {
    int pos = 0;
    double max = x[0];

    for (size_t i = 0; i < OUTPUTS; i++) {
        if (x[i] > max) {
            max = x[i];
            pos = i;
        }
    }
    return pos;
}

double getmax(double *x) {
    double max = x[0];

    for (size_t i = 0; i < OUTPUTS; i++) {
        if (x[i] > max)
            max = x[i];
    }
    return max;
}

void save_params(NeuralNetwork *nn) {
    FILE *file_wIH = fopen("wHI.w", "w");
    if (file_wIH == NULL) {
        perror("Erreur lors de l'ouverture du fichier wIH.w");
        return;
    }
    for (size_t i = 0; i < HIDDEN; i++) {
        for (size_t j = 0; j < INPUTS; j++) {
            fprintf(file_wIH, "%lf\n", nn->weights_1[i][j]);
        }
        fprintf(file_wIH, "\n");
    }
    fclose(file_wIH);

    FILE *file_wHO = fopen("wHO.w", "w");
    if (file_wHO == NULL) {
        perror("Erreur lors de l'ouverture du fichier wHO.w");
        return;
    }
    for (size_t i = 0; i < OUTPUTS; i++) {
        for (size_t j = 0; j < HIDDEN; j++) {
            fprintf(file_wHO, "%lf\n", nn->weights_2[i][j]);
        }
        fprintf(file_wHO, "\n");
    }
    fclose(file_wHO);

    FILE *file_bIH = fopen("bIH.b", "w");
    if (file_bIH == NULL) {
        perror("Erreur lors de l'ouverture du fichier bIH.b");
        return;
    }
    for (size_t i = 0; i < HIDDEN; i++) {
        fprintf(file_bIH, "%lf\n", nn->bias_1[i]);
    }
    fclose(file_bIH);

    FILE *file_bHO = fopen("bHO.b", "w");
    if (file_bHO == NULL) {
        perror("Erreur lors de l'ouverture du fichier bHO.b");
        return;
    }
    for (size_t i = 0; i < OUTPUTS; i++) {
        fprintf(file_bHO, "%lf\n", nn->bias_2[i]);
    }
    fclose(file_bHO);

    //printf("Les paramètres ont été sauvegardés avec succès.\n");
}

double convertStringToDouble(char *s) {
    int i = 2;
    if (s[0] == '-')
        i++;
    double res = 0;
    res += (double) (s[i] - '0') / 10.;
    res += (double) (s[i + 1] - '0') / 100.;
    res += (double) (s[i + 2] - '0') / 1000.;
    res += (double) (s[i + 3] - '0') / 10000.;
    res += (double) (s[i + 4] - '0') / 100000.;
    res += (double) (s[i + 5] - '0') / 1000000.;

    if (s[0] == '-')
        res *= -1;
    return res;
}


void load_params(NeuralNetwork *nn) {
    FILE *file;
    int i;
    char line[1024];
    file = fopen("wHI.w", "r");
    if (file == NULL) errx(EXIT_FAILURE, "Erreur lors de l'ouverture du fichier wIH.w");

    i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\n') {
            double res = convertStringToDouble(line);

            nn->weights_1[i / INPUTS][i % INPUTS] = res;
            i++;
        }
    }
    fclose(file);
    /* errx(EXIT_FAILURE, "oui");
   for (size_t i = 0; i < HIDDEN; i++) {
        for (size_t j = 0; j < INPUTS; j++) {
            int result = fscanf(file_wIH, "%lf", &nn->weights_1[i][j]);
            if (result == EOF) {
                if (feof(file_wIH)) {
                    fprintf(stderr, "Fin de fichier atteinte prématurément à la position [%zu][%zu].\n", i, j);
                    errx(EXIT_FAILURE, "couldn't open\n");
                } else if (ferror(file_wIH)) {
                    perror("Erreur de lecture dans le fichier wIH.w");
                    errx(EXIT_FAILURE, "couldn't open\n");
                }
                fclose(file_wIH);
                return;
            } else if (result != 1) {
                fprintf(stderr, "Erreur de lecture à la position [%zu][%zu]. Valeur attendue non trouvée.\n", i, j);
                errx(EXIT_FAILURE, "couldn't open\n");
                fclose(file_wIH);
                return;
            }
        }
    }*/
/*
    FILE *file_wHO = fopen("wHO.w", "r");
    if (file_wHO == NULL) {
        perror("Erreur lors de l'ouverture du fichier wHO.w");
        errx(EXIT_FAILURE, "couldn't open\n");
        return;
    }
    for (size_t i = 0; i < OUTPUTS; i++) {
        for (size_t j = 0; j < HIDDEN; j++) {
            if (fscanf(file_wHO, "%lf", &nn->weights_2[i][j]) != 1) {
                perror("Erreur de lecture dans le fichier wHO.w");
                errx(EXIT_FAILURE, "couldn't open\n");
                fclose(file_wHO);
                return;
            }
        }
    }
    fclose(file_wHO);*/

    file = fopen("wHO.w", "r");
    if (file == NULL) errx(EXIT_FAILURE, "Erreur lors de l'ouverture du fichier wHO.w");

    i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\n') {
            double res = convertStringToDouble(line);

            nn->weights_2[i / HIDDEN][i % HIDDEN] = res;
            i++;
        }
    }
    fclose(file);

    file = fopen("bIH.b", "r");
    if (file == NULL) errx(EXIT_FAILURE, "Erreur lors de l'ouverture du fichier bIH.b");

    i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\n') {
            double res = convertStringToDouble(line);

            nn->bias_1[i] = res;
            i++;
        }
    }
    fclose(file);

    file = fopen("bHO.b", "r");
    if (file == NULL) errx(EXIT_FAILURE, "Erreur lors de l'ouverture du fichier bHO.b");

    i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '\n') {
            double res = convertStringToDouble(line);

            nn->bias_2[i] = res;
            i++;
        }
    }
    fclose(file);
    /*
    FILE *file_bIH = fopen("bIH.b", "r");
    if (file_bIH == NULL) {
        perror("Erreur lors de l'ouverture du fichier bIH.b");
        errx(EXIT_FAILURE, "couldn't open\n");
        return;
    }
    for (size_t i = 0; i < HIDDEN; i++) {
        if (fscanf(file_bIH, "%lf", &nn->bias_1[i]) != 1) {
            perror("Erreur de lecture dans le fichier bIH.b");
            errx(EXIT_FAILURE, "couldn't open\n");
            fclose(file_bIH);
            return;
        }
    }
    fclose(file_bIH);
*/
    /*
    FILE *file_bHO = fopen("bHO.b", "r");
    if (file_bHO == NULL) {
        perror("Erreur lors de l'ouverture du fichier bHO.b");
        errx(EXIT_FAILURE, "couldn't open\n");
        return;
    }
    for (size_t i = 0; i < OUTPUTS; i++) {
        if (fscanf(file_bHO, "%lf", &nn->bias_2[i]) != 1) {
            perror("Erreur de lecture dans le fichier bHO.b");
            errx(EXIT_FAILURE, "couldn't open\n");
            fclose(file_bHO);
            return;
        }
    }
    fclose(file_bHO);*/

    //printf("Les paramètres ont été chargés avec succès.\n");
}

int predict(NeuralNetwork *nn, double *image) {
    nn->inputs = image;
    forward_propagation(nn);
    int predi = getposmax(nn->output);
    printf("Prediction : %i\n",predi);

    return predi;
}

