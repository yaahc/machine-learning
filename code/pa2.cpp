#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "PatternSetRef.h"

using namespace std;

//regular old driver function. Did my best to emulate the output from the trace. did spaces instead of tabs.
int main(int argc, char* argv[]) {
    if(argc != 2) { //wrong number of arguments
        cout << "usage:\t./pa2 FILE\n";
        return -1;
    }
    //read the settings file from argv[1]
    FILE * settingsFile;
    settingsFile = fopen(argv[1], "r");

    //over complicating the reading because I was bored and wanted to be able
    //to comment my config.txt file
    int lineNum=0, k, din, dOutput, nTrain, nTest;
    char dMetric, oMethod;
    char * input = new char[128];
    char * testing = new char[128];
    char * training = new char[128];
    char * output = new char[128];
    
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
                        strcpy(training, input);
                        break;
                    case 8: //n-test
                        nTest = atoi(input);
                        break;
                    case 9: //testing set file name
                        strcpy(testing, input);
                        break;
                    case 10: //output file name
                        strcpy(output, input);
                        break;
                    default: //should go here if lineNum goes above 7
                        cout << "unexpected input line, are you sure this config file is correctly formated\"" << input << "\"" << endl;
                        break;
                }
            }
        }

    }

    printf("k:%d din:%d dout:%d nTrain:%d nTest:%d\n", k, din, dOutput, nTrain, nTest);

    PatternSetRef* patterns;
    patterns = new PatternSetRef(nTrain, din, dOutput);
    ifstream training2;
    training2.open(training);
    if(training2.is_open()) {
        training2 >> *patterns;
    }
    return 0;
}
