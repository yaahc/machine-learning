/*
 * implementation of a node of a decision learning tree
 *
 * John Lusby
 *
 */

#include <list>
using namespace std;

typedef struct attribute {
    int index;
    double gain;
    int num_attribute;
} attribute;

typedef struct decisionTreeNode {
    list<attribute> attributes;
    attribute decision;
    int* example_indexes;
    int num_examples;
    int num_positive;
    int label;
    PatternSet* pset;
    decisionTreeNode* childP;
    decisionTreeNode* childN;
} DTreeNode;
