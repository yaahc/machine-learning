/**
 *
 * PatternSet class (abstract? not really in the actual definition of an abstract class with virtual
 * members that must be implemented and it in its self cant be instantiated. But it is an abstract data 
 * type in the sense where it represents an idea or object, it tells the code how to make a thing that is 
 * a pattern set)
 *
 * The Rand code and comments were copied from the cplusplus reference website example code.
 *
 * there are a few instance of variable names where I just copied a line or two from the reference examples
 * on gsl and didn't change the names because it was un important, only example I can think of are the rand
 * and eigenvector calculating codes. Really isn't important though but I thought I'd mention it because I
 * don't want to get marked down for inadequate comments and I don't really have much stuff to write about 
 * that doesn't sound redudant or obvious. 
 *
 * Made By John Lusby - 2/19/2013 for machine learning Project 1
 * Its beautiful if you ask me. Except for those sorter classes. those are ugly as eff
 * Only assistance received was from references and wikipedia, learning how to calculate PCA and what not
 * No help was received in coding except looking up references on how to use functions and junk.
 */

#include "PatternSet.h"
#include <cstdio>

//da constructa
PatternSet::PatternSet(int numPatterns, int dimInput, int dimOutput) {
    this->numPatterns = numPatterns;
    this->dimInput = dimInput;
    this->dimOutput = dimOutput;

    /* initialize random seed: */
    srand (time(NULL));

    if(numPatterns > 0 && dimInput > 0)
        inputVectors = gsl_matrix_alloc(numPatterns, dimInput);

    if(numPatterns > 0 && dimOutput > 0)
        outputVectors = gsl_matrix_alloc(numPatterns, dimOutput);
}

//destructor. Might need work. Unimportant? I think so
PatternSet::~PatternSet() {
    if(dimInput > 0)
        delete inputVectors;
    if(dimOutput > 0) 
        delete outputVectors;
    //delete orderSequence; //Do i need to deallocate this stuff at all? I barely understand C++ class destructors. Always worked in C
}

//read a collection of input output vectors from a file and initialize the data matrix with said values
void PatternSet::ReadPatterns(FILE * input) {
    char line[256];
    char * curVal;

    for(int i = 0; i < numPatterns; i++) {
        if(!feof(input)) {
            if(fgets(line, 256, input) != NULL) {
                dout << line;
                curVal = strtok(line, " ");
                int j = 0;
                while(curVal != NULL) {
                    gsl_matrix_set(inputVectors, i, j, atof(curVal));
                    dout << atof(curVal) << std::endl;
                    j++;
                    curVal = strtok(NULL, " ");
                }
            }
        }
    }
}

//writing the collection of patterns to the terminal and to a text file - each row containing one input vector followed by the corresponding output vector - in the order specified by the ordering sequence
void PatternSet::WritePatterns(FILE * output) {
    for(int i = 0; i < numPatterns; i++) {
        for(int j = 0; j < dimInput + dimOutput; j++) {
            if(j < dimInput)
                fprintf(output, "%f ", gsl_matrix_get(inputVectors, orderSequence[i], j));
            else if(j < dimInput + dimOutput)
                fprintf(output, "%f ", gsl_matrix_get(outputVectors, orderSequence[i], j));
        }
        fprintf(output, "\n");
    }
}

//modifies order sequence vector so that the indicies are in random order
void PatternSet::GenerateRandomOrderingSequence() {
    dout << "Staarting GROS\n";
    std::vector<int>::iterator it;

    it = orderSequence.begin();
    it = orderSequence.insert(it, 0);

    for(int i = 1; i < numPatterns; i++) {
        dout << "insert  #" << i << std::endl;
        it = orderSequence.begin();
        dout << orderSequence.size() << std::endl;
        it = orderSequence.insert(it + (rand() % (orderSequence.size() + 1)), i);
    }
}

//modifies orderSequence vector to represent the original order of the input data
void PatternSet::GenerateOriginalOrderingSequence() {
    orderSequence.resize(numPatterns);
    for(int i = 0; i < numPatterns; i++) {
        orderSequence[i] = i;
    }
}

//Local Class used just as an input in algorithms.h's sort function to allow me to sort the indecies vector based on the values of the euclidean distance
//UGLY AS FKKKK
class SortPatternSetEuclidean {
    private:
        std::vector<int> indexes;
        std::vector<double> target;
        gsl_matrix * data;
        int dimInput;
    public:
        SortPatternSetEuclidean(std::vector<int> & indexess, std::vector<double> & targett, gsl_matrix * dataa, int dimInputt) {
            indexes = indexess;
            target = targett;
            data = dataa;
            dimInput = dimInputt;
        }
        bool operator()(int index1, int index2) {
            double sum1 = 0, sum2 = 0;

            for(int i = 0; i < dimInput; i++) {
                sum1 += pow(target[i] - gsl_matrix_get(data, index1, i), 2);
                sum2 += pow(target[i] - gsl_matrix_get(data, index2, i), 2);
            }

            sum1 = sqrt(sum1);
            sum2 = sqrt(sum2);

            return sum1<sum2;
        }
};

//Function to sort orderSequence vector to represent the ascending order of euclidean distance from the target
void PatternSet::GenerateEuclideanOrderingSequence(std::vector<double> & target) {
    GenerateOriginalOrderingSequence();

    std::sort (orderSequence.begin(), orderSequence.end(), SortPatternSetEuclidean(orderSequence, target, inputVectors, dimInput));
}

//Local Class used just as an input in algorithms.h's sort function to allow me to sort the indecies vector based on the values of the angular distance
//UGLY AS FKKKK
class SortPatternSetAngular {
    private:
        std::vector<int> indexes;
        std::vector<double> target;
        gsl_matrix * data;
        int dimInput;
    public:
        SortPatternSetAngular(std::vector<int> & indexess, std::vector<double> & targett, gsl_matrix * dataa, int dimInputt) {
            indexes = indexess;
            target = targett;
            data = dataa;
            dimInput = dimInputt;
        }
        bool operator()(int index1, int index2) {
            double dotProduct1 = 0, dotProduct2 = 0;

            for(int i = 0; i < dimInput; i++) {
                dotProduct1 += target[i]*gsl_matrix_get(data, index1, i);
                dotProduct2 += target[i]*gsl_matrix_get(data, index2, i);
            }

            double magnitude1 = 0, magnitude2 = 0, magnitude3 = 0;

            for (int i = 0; i < dimInput; i++) {
                magnitude1 += pow(gsl_matrix_get(data, index1, i), 2);
                magnitude2 += pow(gsl_matrix_get(data, index2, i), 2);
                magnitude3 += pow(target[i], 2);
            }

            magnitude1 = sqrt(magnitude1);
            magnitude2 = sqrt(magnitude2);
            magnitude3 = sqrt(magnitude3);

            double angle1 = acos(dotProduct1 / (magnitude1*magnitude3));
            double angle2 = acos(dotProduct2 / (magnitude2*magnitude3));

            return angle1<angle2;
        }
};

//Function to sort orderSequence vector to represent the ascending order of angular distance from the target
void PatternSet::GenerateAngularOrderingSequence(std::vector<double> & target) {
    GenerateOriginalOrderingSequence();

    std::sort (orderSequence.begin(), orderSequence.end(), SortPatternSetAngular(orderSequence, target, inputVectors, dimInput));
}

PatternSet * PatternSet::ProjectOntoPCA() {
    double covar;
    //calculate the covariance matrix

    gsl_vector * meanVector = gsl_vector_alloc(dimInput);
    for(int i = 0; i < dimInput; i++) {
        gsl_vector_view col = gsl_matrix_column(inputVectors, i);
        gsl_vector_set(meanVector, i, gsl_stats_mean(col.vector.data, col.vector.stride, numPatterns));
    }

    gsl_matrix * covarianceMatrix = gsl_matrix_alloc(dimInput, dimInput);
    for(int i = 0; i < dimInput; i++) {
        gsl_vector_view v1 = gsl_matrix_column(inputVectors, i);
        for(int j = 0; j < dimInput; j++) {
            gsl_vector_view v2 = gsl_matrix_column(inputVectors, j);
            covar = gsl_stats_covariance_m(v1.vector.data, v1.vector.stride, v2.vector.data, v2.vector.stride, numPatterns, gsl_vector_get(meanVector, i), gsl_vector_get(meanVector, j));
            gsl_matrix_set(covarianceMatrix, i, j, covar);
            dout << covar << std::endl;
        }
    }
    
    //calculate the eigenvectors and variable names were based off of the example code for gsl eigensystems
    gsl_vector *eval = gsl_vector_alloc(dimInput); //eigenValues
    gsl_matrix *evec = gsl_matrix_alloc(dimInput, dimInput); //eigenVectors
    
    //workspace for the eigen calculations, whatever that means!
    gsl_eigen_symmv_workspace * w = gsl_eigen_symmv_alloc(dimInput);
    
    //calculate the eigen vectors
    gsl_eigen_symmv(covarianceMatrix, eval, evec, w);

    //sort the eigen vectors
    gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_DESC);

    //gsl_matrix_fprintf(stdout, evec, "%f");
    
    //gsl_vector_fprintf(stdout, eval, "%f");

    //project the values of the data from my input pattern to the new PatternSet based on the eigenVectors calculated above. Lots of transposes, might be able to make this less ugly if I was more familiar with GSL
    PatternSet * output = new PatternSet(numPatterns, dimInput, dimOutput);
    output->GenerateOriginalOrderingSequence();
    
    gsl_matrix * Projected = gsl_matrix_alloc(dimInput, numPatterns);

    gsl_matrix * adjustedInput = gsl_matrix_alloc(numPatterns, dimInput);
    
    for(int i = 0; i < numPatterns; i++) {
        for(int j = 0; j < dimInput; j++) {
            gsl_matrix_set(adjustedInput, i, j, gsl_matrix_get(inputVectors, i, j) - gsl_vector_get(meanVector, j));
        }
    }

    //printf("%d == %d %d == %d %d == %d", Projected->size1, evec->size1, Projected->size2, inputVectors->size2, evec->size1, inputVectors->size2);

    gsl_blas_dgemm(CblasTrans, CblasTrans, 1.0, evec, adjustedInput, 1.0, Projected);

    gsl_matrix_transpose_memcpy(output->inputVectors, Projected);

    //output->WritePatterns(stdout);

    return output;
}
