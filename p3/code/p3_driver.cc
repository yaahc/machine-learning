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
#include <cstring>

#include <gsl/gsl_vector.h>

#include "patterns.h"
#include "DTreeNode.h"


using namespace std;


//
// Utility Functions
//
//

// trim -- Destructively remove whitespace from either side of a string.
string& trim(string& s) {
    string whtspc = " ";

    s = s.erase(s.find_last_not_of(whtspc) + 1);
    s = s.erase(0, s.find_first_not_of(whtspc));
    return (s);
}

void ID3(DTreeNode* node);
void printDT(DTreeNode* root, int depth);

//
// Main Driver Program
//

int main(int argc, char** argv) {
    PatternSet* pset;
    ifstream config_file_str;
    ifstream input_file_str;
    int input_dimensionality;
    int num_training;
    string training_file_name;
    int num_testing;
    string testing_file_name;
    string output_file_name;

    string pattern_file_name;

    // Check number of arguments ...
    if (argc != 2)
        cerr << argv[0] << " warning: incorrect number of arguments" << endl;

    // Read parameters ...
    config_file_str.open(argv[1]);
    if(!config_file_str.is_open()) {
        cerr << argv[0] << " error: could not open config file." << endl;
        return(-1);
    }
    config_file_str >> input_dimensionality;
    config_file_str >> num_training;
    config_file_str >> training_file_name;
    config_file_str >> num_testing;
    config_file_str >> testing_file_name;
    config_file_str >> output_file_name;

    // Make pattern set ...
    pset = new PatternSet(num_training, input_dimensionality, 1);

    // Open the pattern set file ...
    input_file_str.open(trim(training_file_name).c_str());
    if (!input_file_str.is_open()) {
        cerr << argv[0] << " error:  cannot open specified pattern file." << endl;
        return (-1);
    }

    // Read the patterns ...
    input_file_str >> (*pset);

    // Close the pattern set file ...
    if (input_file_str.is_open())
        input_file_str.close();

    //create decision learning tree
    //create a root node fot the tree
    DTreeNode Root;
    Root.pset = pset;
    Root.example_indexes = new int[num_training];
    Root.num_examples = num_training;
    for(int i = 0; i < num_training; i++)
        Root.example_indexes[i] = i;
    attribute temp;
    for(int i = 0; i < input_dimensionality; i++) {
        temp.index = i;
        temp.gain = 0;
        Root.attributes.push_back(temp);
    }

    //if all examples are positive return the single-node tree root with label =+
    //if all examples are negative return the single-node tree root, with label =-
    //if Attributes is empty terutn the single-node tree root with label = most common value of target_attribute in examples

    ID3(&Root);
    printDT(&Root, 0);

    return (0);
}
//all of these are coded under the assumption that the values are all always binary.
//it compensates for nonbinary input by assuming any number greater than .5 is 1 and anything less is 0
double slog(double num) {
    if(num == 0)
        return 0;
    else
        return log(num)/log(2);
}

double entropy(double numP, double numN) {
    return (0 - (numP/(numP+numN) * slog(numP/(numP+numN))) - (numN/(numP+numN) * slog(numN/(numP+numN))));
}

//populates all the gain values in my attributes list for the vectors indicated by example_indexes
void calc_gains(DTreeNode* node) {
    list<attribute>::iterator it = node->attributes.begin();
    while(it != node->attributes.end()) {
        //calculate entropy of positive examples
        double cap = 0;
        double can = 0;
        double cnap = 0;
        double cnan = 0;
        gsl_vector* curr = gsl_vector_alloc(17);//TODO access the correct size dynamicly
        for(int i = 0; i < node->num_examples; i++) {
            node->pset->full_pattern(node->example_indexes[i], curr);
            //gsl_vector_get(curr, it->index) <- if it has this attribute or not
            //gsl_vector_get(curr, 17) <- the output value of the vector TODO find this dynamically
            if(gsl_vector_get(curr, it->index) >= 0.5) { //has the attribute
                if(gsl_vector_get(curr, 16) >= 0.5) { //positive example
                    cap++;
                } else {
                    can++;
                }
            } else {
                if(gsl_vector_get(curr, 16) >= 0.5) {
                    cnap++;
                } else {
                    cnan++;
                }
            }
        }
        it->num_attribute = (cap+can);
        node->num_positive = cap + cnap;
        it->gain = entropy(cap+cnap, can+cnan);
        it->gain -= (cap+can)/node->num_examples*entropy(cap, can);
        it->gain -= (cnap+cnan)/node->num_examples*entropy(cnap, cnan);
        /* cout << "gain " << it->gain << " "; */
        /* cout << " that should be the values for attribute @ index " << it->index << endl; */
        it++;
    }
}

list<attribute>::iterator best_attribute(DTreeNode* node) {
    list<attribute>::iterator it = node->attributes.begin();
    list<attribute>::iterator best = it;
    while(it != node->attributes.end()) {
        if(it->gain > best->gain)
            best = it;
        it++;
    }
    return best;
}

void printDT(DTreeNode* root, int depth) {
    if(root->label == 1) {
        for(int i = 0; i < depth; i++)
            cout << "  ";
        cout << "  TRUE" << endl;
    } else if(root->label == 0) {
        for(int i = 0; i < depth; i++)
            cout << "  ";
        cout << "  FALSE" << endl;
    } else {
        if(root->childN) {
            for(int i = 0; i < depth; i++)
                cout << "  ";
            cout << root->decision.index + 1;
            cout << " false then" << root->num_positive << " " << root->num_examples <<"\n";
            printDT(root->childN, depth+1);
        }
        if(root->childP) {
            for(int i = 0; i < depth; i++)
                cout << "  ";
            cout << root->decision.index + 1;
            cout << " true then" << root->num_positive << " " << root->num_examples <<"\n";
            printDT(root->childP, depth+1);
        }    }
}

void ID3(DTreeNode* node) {
    calc_gains(node);
    if(node->num_positive == node->num_examples) {
        node->label = 1;
        cout << "1\n";
        return;
    } else if(node->num_positive == 0) {
        node->label = 0; 
        cout << "0\n";
        return;
    } else {
        node->label = -1;
    }
    //pick the attribute A from attributes that best classifies example_indexes
    //set the decision attribute for node to A
    node->decision = *best_attribute(node);
    cout << "decision:" << node->decision.index << endl;
    for(int i = 0; i < node->num_examples; i++) {
        cout << node->example_indexes[i] << " ";
    }
    cout << endl;

    //split* the set based on the attribute
    int * positive = new int[node->decision.num_attribute];
    int * negative = new int[node->num_examples - node->decision.num_attribute];
    gsl_vector* curr = gsl_vector_alloc(17);//TODO access the correct size dynamicly
    //gsl_vector_get(curr, it->index) <- if it has this attribute or not
    //gsl_vector_get(curr, 17) <- the output value of the vector TODO find this dynamically
    for(int i = 0, j = 0; j+i < node->num_examples;) {
        node->pset->full_pattern(node->example_indexes[i+j], curr);
        int index;
        if(gsl_vector_get(curr, node->decision.index)) {
            /* cout << "p" << node->example_indexes[i+j] << " "; */
            positive[i++] = node->example_indexes[i+j];
        } else {
            /* cout << "n" << node->example_indexes[i+j] << " "; */
            negative[j++] = node->example_indexes[i+j];
        }
    }
    //construct child nodes
    if(node->decision.num_attribute > 0) {
        node->childP = new DTreeNode;
        node->childP->example_indexes = positive;
        node->childP->num_examples = node->decision.num_attribute;
        node->childP->pset = node->pset;
        node->childP->attributes = node->attributes;
        node->childP->attributes.erase(best_attribute(node->childP));
        ID3(node->childP);
    } else {
        cout << "TODO" << endl;
    }

    if(node->num_examples - node->decision.num_attribute > 0) {
        node->childN = new DTreeNode;
        node->childN->example_indexes = negative;
        node->childN->num_examples = node->num_examples - node->decision.num_attribute;
        node->childN->pset = node->pset;
        node->childN->attributes = node->attributes;
        node->childN->attributes.erase(best_attribute(node->childN));
        ID3(node->childN);
    } else {
        cout << "TODO" << endl;
    }
    /* cout << endl; */
    //for each possible value vi of A
    //  add a new tree branch
    //  let examples vi be the subset of examples that have value vi for A
    //  if examples vi is empty
    //      then below this new branch add a leaf node with label = most common value of Target_attribute in examples
    //      else add a new subtree below this branch
    //          ID3()
}
