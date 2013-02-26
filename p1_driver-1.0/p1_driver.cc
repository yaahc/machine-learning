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
  gsl_vector* target_vector;

  // Check number of arguments ...
  if (argc > 1)
    cerr << argv[0] << " warning:  ignoring command line arguments." << endl;
  // Read parameters ...
  cout << "Enter the input vector dimensionality:" << endl;
  cin >> input_dimensionality;
  if ((input_dimensionality < 1) || !cin) {
    cerr << argv[0] << " error:  cannot parse input dimensionality." << endl;
    return (-1);
  }
  cout << "Enter the number of patterns:" << endl;
  cin >> number_of_patterns;
  if ((number_of_patterns < 1) || !cin) {
    cerr << argv[0] << " error:  cannot parse number of patterns." << endl;
    return (-1);
  }
  // Make pattern set ...
  pset = new PatternSet(number_of_patterns, input_dimensionality, 0);
  // Read pattern set file name ...
  cout << "Enter pattern file pathname:" << endl;
  pattern_file_name = "";
  while (pattern_file_name.length() == 0) {
    getline(cin, pattern_file_name);
    pattern_file_name = trim(pattern_file_name);
  }
  // Open the pattern set file ...
  input_file_str.open(pattern_file_name.c_str());
  if (!input_file_str.is_open()) {
    cerr << argv[0] << " error:  cannot open specified pattern file." << endl;
    return (-1);
  }
  // Read the patterns ...
  input_file_str >> (*pset);
  // Close the pattern set file ...
  if (input_file_str.is_open())
    input_file_str.close();
  // Read the target vector ...
  target_vector = gsl_vector_alloc(input_dimensionality);
  cout << "Enter the target vector, with elements separated by whitespace:"
       << endl;
  gsl_vector_fscanf(stdin, target_vector);
  // Output the input vectors in a random order ...
  pset->set_permute_flag();
  pset->permute_patterns();
  cout << endl;
  cout << "Randomly permuted patterns:" << endl << (*pset);
  // Output the input vectors in their original order ...
  pset->clear_permute_flag();
  cout << endl;
  cout << "Patterns in their original order:" << endl << (*pset);
  // Output the vectors in Euclidean distance order ...
  pset->set_permute_flag();
  (void) pset->sort_euclidean(target_vector);
  cout << endl;
  cout << "Patterns in order of growing Euclidean distance from target vector:"
       << endl << (*pset);
  // Output the vectors in angular distance order ...
  (void) pset->sort_angular(target_vector);
  cout << endl;
  cout << "Patterns in order of growing angular distance from target vector:"
       << endl << (*pset);
  pset->clear_permute_flag();
  // Compute the PCA projections ...
  PatternSet projected_pset = pset->pca_project();
  projected_pset.clear_permute_flag();
  cout << endl;
  cout << "PCA projected patterns in their original order:" << endl
       << projected_pset;
  // Deallocate storage ...
  delete pset;
  gsl_vector_free(target_vector);
  // Done ...
  cout << endl << "Done." << endl;
  return (0);
}

