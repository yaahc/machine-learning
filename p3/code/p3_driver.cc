//
// David C. Noelle - Mon Feb 23 22:20:22 PST 2009
//

/*
 * p3_driver / implementation of decision tree learner.
 *
 * Based on my p2_driver which is in turn based on David C. Noelle's p1_driver solution
 *
 * John Lusby - Sun 3/17/2013
 *
 * uses David Noelle's pattern.h and pattern.cc with very few modifications (I'm actually not even sure if the modifications I made during project 3 are still relevant, since the only one that comes to mind was for the sorting portion, where I wanted to be able to reuse the distance calculations without having to recalculate them.
 *
 */


#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <cstring>

#include <gsl/gsl_vector.h>

#include "patterns.h"
#include "DTreeNode.h" //data structures for my decision tree. I probably could reimplement it in a proper class format but I chose to do it procedurally because It is more natural / easier. Requires less planning. 


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

//headers for functions that I declared below main but wanted to call in main.
void ID3(DTreeNode* node);
void printDT(DTreeNode* root, int depth);
int classify_pattern(DTreeNode* Root, gsl_vector* pattern); 

//
// Main Driver Program
//

int main(int argc, char** argv) {
    PatternSet* pset;
    PatternSet* testingpset;
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
    for(int i = 0; i < num_training; i++) //fill out index list
        Root.example_indexes[i] = i;
    attribute temp;
    for(int i = 0; i < input_dimensionality; i++) { //fill out attributes list
        temp.index = i;
        temp.gain = 0;
        Root.attributes.push_back(temp);
    }

    //make tree
    ID3(&Root);
    //print tree
    printDT(&Root, 0);

    //classify testing examples
    // Make pattern set ...
    testingpset = new PatternSet(num_testing, input_dimensionality, 1);

    // Open the pattern set file ...
    input_file_str.open(trim(testing_file_name).c_str());
    if (!input_file_str.is_open()) {
        cerr << argv[0] << " error:  cannot open specified pattern file." << endl;
        return (-1);
    }

    // Read the patterns ...
    input_file_str >> (*testingpset);

    // Close the pattern set file ...
    if (input_file_str.is_open())
        input_file_str.close();

    //classify values and output results to output_file
    double errors = 0;
    gsl_vector* curr = gsl_vector_alloc(input_dimensionality+1);
    ofstream output_file_str(trim(output_file_name).c_str());
    for(int i = 0; i < num_testing; i++) {
        testingpset->full_pattern(i, curr);
        for(int j = 0; j < input_dimensionality; j++) {
            output_file_str << gsl_vector_get(curr, j) << " ";
        }
        output_file_str << classify_pattern(&Root, curr) << " " << gsl_vector_get(curr, input_dimensionality) << " "; //This line calls the function to travel down the tree to the leaf node that classifies the pattern
        if(classify_pattern(&Root, curr) == gsl_vector_get(curr, input_dimensionality))
            output_file_str << 0 << endl;
        else {
            output_file_str << 1 << endl;
            errors++;
        }
    }
    output_file_str << errors/num_testing << endl;
    output_file_str.close();
    return (0);
}

//use label variable to differentiate between leafs and nodes
//1 is true leaf, 0 is false leaf, -1 is non leaf node
int classify_pattern(DTreeNode* root, gsl_vector* pattern) { 
    if(root->label == 1)
        return 1;
    else if(root->label == 0)
        return 0;
    else {
        if(gsl_vector_get(pattern, root->decision.index)) //pick next node based on decision
            return classify_pattern(root->childP, pattern);
        else
            return classify_pattern(root->childN, pattern);
    }
}

//all of these are coded under the assumption that the values are all always binary.
//it compensates for nonbinary input by assuming any number greater than .5 is 1 and anything less is 0

//calculate the log base 2 given that we're assuming log(0) = 0
double slog(double num) {
    if(num == 0)
        return 0;
    else
        return log(num)/log(2);
}

//calculate the entropy of set with numP positive elements and numN negative elements
double entropy(double numP, double numN) {
    return (0 - (numP/(numP+numN) * slog(numP/(numP+numN))) - (numN/(numP+numN) * slog(numN/(numP+numN))));
}

//populates all the gain values in my attributes list for the vectors indicated by example_indexes
void calc_gains(DTreeNode* node) {
    list<attribute>::iterator it = node->attributes.begin(); //for each attribute that is still under consideration
    while(it != node->attributes.end()) {
        //calculate entropy of positive examples
        double cap = 0; //count attributes positive
        double can = 0; //count attributes negative
        double cnap = 0; //count non attributes positive
        double cnan = 0; //count non attributes negative
        gsl_vector* curr = gsl_vector_alloc(node->pset->pattern_size());
        for(int i = 0; i < node->num_examples; i++) {
            node->pset->full_pattern(node->example_indexes[i], curr);
            if(gsl_vector_get(curr, it->index) >= 0.5) //has the attribute
                if(gsl_vector_get(curr, node->pset->pattern_size()-1) >= 0.5)
                    cap++;
                else
                    can++;
            else //doesn't have that attribute
                if(gsl_vector_get(curr, node->pset->pattern_size()-1) >= 0.5)
                    cnap++;
                else
                    cnan++;
        }
        //save numbers useful for other calculations into variables for node / attributes
        it->num_attribute = (cap+can);
        node->num_positive = cap + cnap;
        it->gain = entropy(cap+cnap, can+cnan);
        it->gain -= (cap+can)/node->num_examples*entropy(cap, can);
        it->gain -= (cnap+cnan)/node->num_examples*entropy(cnap, cnan);
        it++;
    }
}

//returns an iterater pointing to the attribute with the highest gain at the current
//node. I used an iterator because it was the easiest thing to get to work with remove 
//on a list
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

//print a visual representation of the decision tree to stdout
void printDT(DTreeNode* root, int depth) {
    if(root->label == 1) { //this is a leaf node for TRUE
        for(int i = 0; i < depth; i++)
            cout << "  ";
        cout << "  TRUE" << endl;
    } else if(root->label == 0) { //this is a leaf node for FALSE
        for(int i = 0; i < depth; i++)
            cout << "  ";
        cout << "  FALSE" << endl;
    } else { //this is a non leaf node. We must go deeper...
        if(root->childN) {
            for(int i = 0; i < depth; i++)
                cout << "  ";
            cout << root->decision.index + 1;
            cout << " false then\n";
            printDT(root->childN, depth+1);
        }
        if(root->childP) {
            for(int i = 0; i < depth; i++)
                cout << "  ";
            cout << root->decision.index + 1;
            cout << " true then\n";
            printDT(root->childP, depth+1);
        }    }
}

//not a super easy to use stand alone version of ID3. But if you set up the root node
//correctly it recusively fills out the children to form a Decision learning tree
void ID3(DTreeNode* node) {
    //calculate the gain and other assorted math-y values based on examples and attributes still under consideration at this node
    calc_gains(node);
    //break conditions
    if(node->num_positive == node->num_examples) { //all of the elements under consideration have a positive output value
        node->label = 1;
        cout << "1\n";
        return;
    } else if(node->num_positive == 0) { //all of the elements have negative output
        node->label = 0; 
        cout << "0\n";
        return;
    } else if(node->attributes.size() == 0) { //there are still elements, but no attributes this is an edge case that shouldnt occur in our simple implementaion (unless I'm stupid)
        cout << "no attributes" << endl;
        //I dont think it gets here
    } else { //currently have multiple elements with non zero entropy to classify. Recurse
        node->label = -1;
    }
    //pick the attribute A from attributes that best classifies example_indexes
    //set the decision attribute for node to A
    node->decision = *best_attribute(node);
    //decision is 0 based. For the print I offset it by 1 to make it match the log we were given in example
    //this prints the current best attribute to decide on
    cout << "decision:" << node->decision.index << endl;
    //this prints all the elements still under consideration at this node
    for(int i = 0; i < node->num_examples; i++) {
        cout << node->example_indexes[i] << " ";
    }
    cout << endl;

    //split* the set based on the attribute
    //I'm using arrays of indexes to indicate which elements are under consideration. Simple, maybe too simple. works fine though
    int * positive = new int[node->decision.num_attribute];
    int * negative = new int[node->num_examples - node->decision.num_attribute];
    gsl_vector* curr = gsl_vector_alloc(node->pset->pattern_size());
    for(int i = 0, j = 0; j+i < node->num_examples;) {
        node->pset->full_pattern(node->example_indexes[i+j], curr);
        int index;
        if(gsl_vector_get(curr, node->decision.index)) {
            positive[i++] = node->example_indexes[i+j];
        } else {
            negative[j++] = node->example_indexes[i+j];
        }
    }

    //construct child nodes
    //Since all the attributes have binary value I hard coded a case for positive and negative values. 
    if(node->decision.num_attribute > 0) {
        node->childP = new DTreeNode; //copies a bunch of values from parent then modifies them slightly to create a node representing the positive value of the attribute
        node->childP->example_indexes = positive;
        node->childP->num_examples = node->decision.num_attribute;
        node->childP->pset = node->pset;
        node->childP->attributes = node->attributes;
        node->childP->attributes.erase(best_attribute(node->childP));
        ID3(node->childP);
    } else { //This is unimportant since our examples are all binary and I hard coded an iteration for both 1 and 0. If it was the case that you could have a dynamic range of values for the attribute then I would need to handle cases where none of the patterns have the value vi for the attribute, in which case it would guess from the most likely value based on the output from the examples it does have. 
        /* cout << "TODO"; */
        /* cout << node->decision.index + 1 << " " << node->decision.gain << endl; */
    }

    //negative attribute version of the above
    if(node->num_examples - node->decision.num_attribute > 0) {
        node->childN = new DTreeNode;
        node->childN->example_indexes = negative;
        node->childN->num_examples = node->num_examples - node->decision.num_attribute;
        node->childN->pset = node->pset;
        node->childN->attributes = node->attributes;
        node->childN->attributes.erase(best_attribute(node->childN));
        ID3(node->childN);
    } else {
        /* cout << "TODO"; */
        /* cout << node->decision.index + 1 << " " << node->decision.gain << endl; */
    }
}
