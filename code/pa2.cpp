#include <iostream>
#include <cstdlib>
#include <cstring>
#include "PatternSet.h"

using namespace std;

//regular old driver function. Did my best to emulate the output from the trace. did spaces instead of tabs.
int main(int argc, char* argv[]) {

    //read the settings file from argv[1]
    FILE * settingsFile;
    settingsFile = fopen(argv[1], "r");

    //over complicating the reading because I was bored and wanted to be able
    //to comment my config.txt file
    int lineNum=0, k, din, dOutput, nTrain, nTest;
    char dMetric, oMethod;
    FILE * training, *testing, *output;
    char * input = new char[128];
    
    while(!feof(settingsFile)) {
        memset(input, 0, 128);
        if(fgets(input, 128, settingsFile)) {
            if(input[0] != '#') {
                lineNum++;
                switch(lineNum) {
                    case 1: //k
                        k = atoi(input);
                        break;
                    case 2: //din
                        din = atoi(input);
                        break;
                    case 3: //dout
                        dOutput = atoi(input);
                        break;
                    case 4: //distance metric
                        dMetric = input[0];
                        break;
                    case 5: //output method
                        oMethod = input[0];
                        break;
                    case 6: //n-train
                        nTrain = atoi(input);
                        break;
                    case 7: //training set file name
                        training = fopen(input, "r");
                        break;
                    case 8: //n-test
                        nTest = atoi(input);
                        break;
                    case 9: //testing set file name
                        testing = fopen(input, "r");
                        break;
                    case 10: //output file name
                        output = fopen(input, "w");
                        break;
                    default: //should go here if lineNum goes above 7
                        cout << "unexpected input line, are you sure this config file is correctly formated\"" << input << "\"" << endl;
                        break;
                }
            }
        }

    }

    printf("k:%d din:%d dout:%d nTrain:%d nTest:%d\n", k, din, dOutput, nTrain, nTest);

    /* //create pattern set of predescribed dimensionality */
    /* PatternSet relevantName1(numPatterns, inputDimensionality, 0); */

    /* //read input file name */
    /* cout << "Enter pattern file pathname:" << endl; */
    /* char patternsFileName[100]; */
    /* cin >> patternsFileName; */
    /* FILE * patterns; */
    /* patterns = fopen(patternsFileName, "r"); */

    /* relevantName1.ReadPatterns(patterns); */

    /* //read target vector */
    /* cout << "Enter the target vector (dim:" << inputDimensionality << "), with elements separated by whitespace:" << endl; */
    /* vector<double> target(inputDimensionality); */

    /* double curDim; */
    /* for(int i = 0; i < inputDimensionality; i++) { */
    /*     cin >> curDim; */
    /*     target[i] = curDim; */
    /* } */

    /* dout << "test" << endl; */

    /* //random reorder for input vectors in the pattern set */
    /* relevantName1.GenerateRandomOrderingSequence(); */

    /* //Writes to terminal, the input vectors in the new random order */
    /* cout << "\n\nRandom ordering" << endl; */
    /* relevantName1.WritePatterns(stdout); */

    /* //writes to the terminal the input vectors in their originally read order */
    /* cout << "\n\nOriginal Ordering" << endl; */
    /* relevantName1.GenerateOriginalOrderingSequence(); */
    /* relevantName1.WritePatterns(stdout); */

    /* //writes to the terminal the input vectors in ascending order of their euclidean distance from the target vector */
    /* cout << "\n\nEuclidean Ordering" << endl; */
    /* relevantName1.GenerateEuclideanOrderingSequence(target); */
    /* relevantName1.WritePatterns(stdout); */

    /* //Writes to the terminal the input vectors in ascending order of their angular distance from the target vector */
    /* cout << "\n\nAngular Ordering" << endl; */
    /* relevantName1.GenerateAngularOrderingSequence(target); */
    /* relevantName1.WritePatterns(stdout); */

    /* //Calculates the principal components of the input vectors and produces a new pattern set with each input vector replaced with its projection onto the principal component axes, with the components sorted in order (i.e., the first element of the projected vectors should correspond to the first principal component and so on) */
    /* PatternSet * relevantName2 = relevantName1.ProjectOntoPCA(); */
    /* cout << "\n\nPCA projected patterns in their original order:" << endl; */
    /* relevantName2->GenerateOriginalOrderingSequence(); */
    /* relevantName2->WritePatterns(stdout); */

    /* //Writes to the terminal, the projected input vectors in the order in which they were read. */
    return 0;
}
