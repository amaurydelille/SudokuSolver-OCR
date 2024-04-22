#include "neuralnetwork.h"
#include "files.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char** argv){

    if (argc < 2)
        errx(EXIT_FAILURE, "Too few arguments");

    srand(time(NULL));
    printf("Dataset done\n");
    NeuralNetwork* nn = malloc(sizeof(NeuralNetwork));
    initialize_neuralnetwork(nn);
    load_params(nn);
    
    printf("Initialization done\n");
    double** train = load_train("Dataset2");
    int* labels = load_labels("labels"); 

    double** test = load_train("../GridSolver/images");
    int* testlabels = load_labels("testlabels");
    
    printf("Parameters done\n");

    if (strcmp(argv[1], "-t") == 0){
    
    //GRADIENT DESCENT / TRAINING
        for(size_t i = 0; i < 5; i++){
   
            double accuracy = 0.0;
            double batch_accuracy = 0;
            for(size_t j = 0; j < SAMPLES; j++) {
                nn->inputs = train[j];

                forward_propagation(nn);

                if (getposmax(nn->output) == labels[j]){
                    accuracy++;
                    batch_accuracy++;     
                }

                back_propagation(nn, labels, j);
                update_params(nn);
                save_params(nn);

                if (j % 1000 == 0){
                    double testaccuracy = 0.0;
                    for(size_t i = 0; i < 81; i++)
                    {
                        nn->inputs = test[j];
                        forward_propagation(nn);
                        if (getposmax(nn) == testlabels[j])
                            testaccuracy++;
                    }

                    printf("- TEST ACCURACY = %f -\n", testaccuracy/81);
                    if (testaccuracy/81 > 0.75)
                        return;
                }
            } 
        
            double epoch_accuracy = (double)accuracy / SAMPLES;
            printf("# Accuracy for EPOCH %zu: %f #\n", i, epoch_accuracy);
        }
    }
    else {
        double* image = path_to_input(argv[1]);
        int result = predict(nn, image);
    }
    
    freeNeuralNetwork(nn);
    free(nn);
    free(train);
    free(labels);
    return 0;

}