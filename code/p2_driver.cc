//
// p1_driver
//
// This is a driver program, intended to provide a simple demonstration of
// of the capabilities of the "PatternSet" class.
//
// David C. Noelle - Mon Feb 23 22:20:22 PST 2009
//



#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include <gsl/gsl_vector.h>

#include "patterns.h"


using namespace std;


//
// Utility Functions
//

// trim -- Destructively remove whitespace from either side of a string.
string& trim(string& s) {
    string whtspc = " ";

    s = s.erase(s.find_last_not_of(whtspc) + 1);
    s = s.erase(0, s.find_first_not_of(whtspc));
    return (s);
}


//
// Main Driver Program
//

int main(int argc, char** argv) {
    int input_dimensionality;
    int number_of_patterns;
    PatternSet* pset;
    string pattern_file_name;
    ifstream input_file_str;
    ifstream config_file_str;

    // Check number of arguments ...
    if (argc != 2)
        cerr << argv[0] << " warning: incorrect number of arguments" << endl;
    // Read parameters ...
    config_file_str.open(argv[1]);
    if(!config_file_str.is_open()) {
        cerr << argv[0] << " error: could not open config file." << endl;
        return(-1);
    }
    int k, output_dimensionality, num_training, num_testing;
    string distance_metric, output_method, training_file, testing_file, output_file;
    config_file_str >> k;
    config_file_str >> input_dimensionality;
    config_file_str >> output_dimensionality;
    config_file_str >> distance_metric;
    config_file_str >> output_method;
    config_file_str >> num_training;
    config_file_str >> training_file;
    config_file_str >> num_testing;
    config_file_str >> testing_file;
    config_file_str >> output_file;

    cout << k << " " << input_dimensionality << " " << output_dimensionality << " " << distance_metric << " " << output_method << " " << num_training << " " << training_file << " " << num_testing << " " << testing_file << " " << output_file << endl;
    // Make pattern set ...
    pset = new PatternSet(num_training, input_dimensionality, output_dimensionality);
    // Open the pattern set file ...
    input_file_str.open(trim(training_file).c_str());
    if (!input_file_str.is_open()) {
        cerr << argv[0] << " error:  cannot open specified pattern file." << endl;
        return (-1);
    }
    // Read the patterns ...
    input_file_str >> (*pset);
    // Close the pattern set file ...
    if (input_file_str.is_open())
        input_file_str.close();
    /* cout << (*pset); */ 

    //clasify the training set... and I wonder
    PatternSet* testingSet = new PatternSet(num_testing, input_dimensionality, output_dimensionality);
    input_file_str.open(trim(testing_file).c_str());
    if(!input_file_str.is_open()) {
        cerr << argv[0] << " error: cannot open specified pattern file." << endl;
        return (-1);
    }
    //read the patterns
    input_file_str >> (*testingSet);
    //close the pattern set file
    if(input_file_str.is_open())
        input_file_str.close();

    gsl_vector* input_vector = gsl_vector_alloc(input_dimensionality);
    gsl_vector* output = gsl_vector_alloc(output_dimensionality);
    gsl_vector* target_vector = gsl_vector_alloc(output_dimensionality);
    gsl_vector* neighbor_i = gsl_vector_alloc(output_dimensionality);
    pset->set_permute_flag();
    ofstream output_file_str(trim(output_file).c_str());
    double totalSSE = 0;
    for(int i = 0; i < num_testing; i++) {
        //input vector
        if(!testingSet->input_pattern(i, input_vector)) {
            cerr << argv[0] << " error: issue copying input vector\n";
        }

        //order the sets by distance metric
        if(distance_metric[0] == 'E') {
            //euclidean distance
            pset->sort_euclidean(input_vector);
        } else if(distance_metric[0] == 'A') {
            //angular distance
            pset->sort_angular(input_vector);
        } else {
            cerr << "wth\n";
        }

        //choose the K-nearest neighbors
        //create classification based on output method
        gsl_vector_scale(output, 0.0);
        double weight_sum = 0;
        bool special_case = false;
        int k_special = 0;
        for(int j = 0; j < k; j++) {
            pset->target_pattern(pset->get_permuted_i(j), neighbor_i);

            //scale the vector if we're doing weighted mean
            if(output_method[0] == 'W') {
                double weight = pow(pset->get_distance(pset->get_permuted_i(j)), 2);
                if(weight != 0 && !special_case) { //as soon as we hit a "special_case" never consider non 0 weighted vectors
                    weight_sum += 1.0/weight;
                    gsl_vector_scale(neighbor_i, 1.0/weight);
                } else if(weight == 0) { 
                    if(!special_case) {
                        special_case = true;
                        gsl_vector_scale(output, 0.0);
                    }
                    k_special++;
                    gsl_vector_add(output, neighbor_i);
                }
            }
            if(!special_case)
                gsl_vector_add(output, neighbor_i);
        }

        //scale the output vector down by the number of vectors used (or sum of weight)
        if(special_case) {
            gsl_vector_scale(output, 1.0/k_special);
        } else if(output_method[0] == 'U' || output_method[0] == 'M') {
            gsl_vector_scale(output, 1.0/k);
        } else if(output_method[0] == 'W') {
            gsl_vector_scale(output, 1.0/weight_sum);
        }

        //calculate Sum Squared Error
        double sse = 0;
        testingSet->target_pattern(i, target_vector);
        for(int j = 0; j < output_dimensionality; j++)
            sse += pow(gsl_vector_get(output, j) - gsl_vector_get(target_vector, j), 2.0);
        totalSSE += sse;

        //Output to the file
        for(int j = 0; j < input_dimensionality; j++)
            output_file_str << gsl_vector_get(input_vector, j) << " ";
        for(int j = 0; j < output_dimensionality; j++)
            output_file_str << gsl_vector_get(output, j) << " ";
        for(int j = 0; j < output_dimensionality; j++)
            output_file_str << gsl_vector_get(target_vector, j) << " ";
        output_file_str << sse << endl;
    }
    output_file_str << totalSSE << endl;
    output_file_str.close();
    /* // Read the target vector ... */
    /* target_vector = gsl_vector_alloc(input_dimensionality); */
    /* cout << "Enter the target vector, with elements separated by whitespace:" */
    /*     << endl; */
    /* gsl_vector_fscanf(stdin, target_vector); */
    /* // Output the input vectors in a random order ... */
    /* pset->set_permute_flag(); */
    /* pset->permute_patterns(); */
    /* cout << endl; */
    /* cout << "Randomly permuted patterns:" << endl << (*pset); */
    /* // Output the input vectors in their original order ... */
    /* pset->clear_permute_flag(); */
    /* cout << endl; */
    /* cout << "Patterns in their original order:" << endl << (*pset); */
    /* // Output the vectors in Euclidean distance order ... */
    /* pset->set_permute_flag(); */
    /* (void) pset->sort_euclidean(target_vector); */
    /* cout << endl; */
    /* cout << "Patterns in order of growing Euclidean distance from target vector:" */
    /*     << endl << (*pset); */
    /* // Output the vectors in angular distance order ... */
    /* (void) pset->sort_angular(target_vector); */
    /* cout << endl; */
    /* cout << "Patterns in order of growing angular distance from target vector:" */
    /*     << endl << (*pset); */
    /* pset->clear_permute_flag(); */
    /* // Compute the PCA projections ... */
    /* PatternSet projected_pset = pset->pca_project(); */
    /* projected_pset.clear_permute_flag(); */
    /* cout << endl; */
    /* cout << "PCA projected patterns in their original order:" << endl */
    /*     << projected_pset; */
    /* // Deallocate storage ... */
    /* delete pset; */
    /* gsl_vector_free(target_vector); */
    /* // Done ... */
    /* cout << endl << "Done." << endl; */
    return (0);
}

