#ifndef PATTERNSET_H
#define PATTERNSET_H

#ifdef DEBUG
#define dout std::cout
#else
#define dout 0 && std::cout
#endif

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
#include <cmath>       /* sqrt, pow */

//header file for PatternSet Class, the big header comment is in the cpp file
class PatternSet {
    private: 
        //the number of patterns in the pattern set
        int numPatterns;

        //the dimensionality of the input vectors
        int dimInput;

        //the dimensionality of the corresponding output vectors
        int dimOutput; //always 0 for this assignment

        //a matrix of input vectors, with the number of rows being the number of patterns and the number of columns being the dimensionality of the input vectors.
        gsl_matrix * inputVectors;// = gsl_matrix_alloc(numPatterns, dimInput);

        //a matrix of output vectors, with the number of rows being the number of patterns and the number of columns being the dimensionality of the output vectors
        gsl_matrix * outputVectors;// = gsl_matrix_alloc(numPatterns, dimOutput);

        //an integer vector indicating the order in which the patterns should be considered(e.g., out-put), called the ordering sequence
        std::vector<int> orderSequence;

        //target sequence vector (I had to include this in order to utilize sort)
        std::vector<double> target;

        //euclidean ordering func
        //bool euclideanComp(int index1, int index2);
        

    public:
        //allocating and deallocating a pattern set
        PatternSet(int numPatterns, int dimInput, int dimOutput);
        ~PatternSet();

        //reading a collection of patterns from the terminal and from a text file - each row containing one input vector followed by the corresponding output vector
        void ReadPatterns(FILE * input);

        //writing the collection of patterns to the terminal and to a text file - each row containing one input vector followed by the corresponding output vector - in the order specified by the ordering sequence
        void WritePatterns(FILE * output);

        //randomly generating a new ordering sequence
        void GenerateRandomOrderingSequence();

        //generate an ordering sequence that corresponds with the original order (1, 2, 3, ...)
        void GenerateOriginalOrderingSequence();

        //generating a new ordering sequence so as to sort the input vectors according to the Euclidean distance to a given target vector
        void GenerateEuclideanOrderingSequence(std::vector<double> & target);

        //generating a new ordering sequence so as to sort the input vectors according to angular distance to a given target vector
        void GenerateAngularOrderingSequence(std::vector<double> & target);

        //generate a copy of a given pattern set with the input vectors projected onto their principal component axes
        PatternSet * ProjectOntoPCA();
};

#endif
