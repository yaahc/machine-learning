//
// patterns.h :  Specification file for a "pattern set" object.
//
// David C. Noelle - Mon Feb 23 14:31:50 PST 2009
// 


// Make sure that this header file is loaded only once ... 
#ifndef PATTERNS_UTILITIES_INCLUDED
#define PATTERNS_UTILITIES_INCLUDED 1


#include <iostream>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>


using namespace std;


// Forward declaration ...
class PatternSetRef;


//
// PatternSetRef Class  --  A collection of training or testing patterns.
//

class PatternSetRef {

    private:

        int n_patterns;           // number of patterns
        int n_inputs;             // number of input values in each pattern
        int n_targets;            // number of target values in each pattern

        gsl_matrix* inputs_m;     // the matrix of input vectors, one per row
        gsl_matrix* targets_m;    // the matrix of target vectors, one per row

        bool permute;             // use the permutation sequence, if true
        int* permutation;         // the pattern indices, randomly permuted

    public:

        // constructors & assignment
        PatternSetRef(int num_pat = 0, int num_inputs = 0, int num_targets = 0);
        PatternSetRef(const PatternSetRef& pset);
        PatternSetRef& operator=(const PatternSetRef& pset);

        // destructor
        ~PatternSetRef();

        // number_of_patterns -- Return the number of patterns in the set, or a
        //                       negative value on error.
        inline int number_of_patterns() const { return n_patterns; }

        // pattern_size -- Return the number of scalar values in each pattern, or
        //                 a negative value on error.
        inline int pattern_size() const { return (n_inputs + n_targets); }

        // number_of_inputs -- Return the number of input values in each pattern,
        //                     or a negative value on error.
        inline int number_of_inputs() const { return n_inputs; }

        // number_of_targets -- Return the number of target values in each pattern,
        //                      or a negative value on error.
        inline int number_of_targets() const { return n_targets; }

        // get_permute_flag -- Return true if the permutation array is to be used
        //                     when writing patterns and performing other similar 
        //                     operations.  Return false if the original order of
        //                     of the patterns is to be used.
        inline bool get_permute_flag() const { return permute; }

        // set_permute_flag -- Specify that the permutation array *should* be used
        //                     when writing patterns and performing other similar
        //                     operations.
        inline void set_permute_flag() { permute = true; }

        // clear_permute_flag -- Specify that the permutation array should *not* be
        //                       used when writing patterns and performing other
        //                       similar operations.
        inline void clear_permute_flag() { permute = false; }

        // get_permuted_i -- Return the index, "i", of the pattern at position "t"
        //                   in the permuted order specified by the permutation
        //                   array.  Return "t" if the permutation array is not
        //                   properly initialized or the permute flag is false.
        //                   Return a negative value on error.
        int get_permuted_i(int t) const;

        // full_pattern -- Copy the pattern with the given index (the "i"th pattern)
        //                 into the given vector, returning a pointer to the 
        //                 vector.  Return NULL on error.
        gsl_vector* full_pattern(int i, gsl_vector* v) const;

        // input_pattern -- Copy the input vector for the "i"th pattern into the 
        //                  given vector, returning a pointer to it.  Return NULL
        //                  if the vector is of the wrong size or if any other
        //                  error arises.
        gsl_vector* input_pattern(int i, gsl_vector* v) const;

        // set_input_pattern -- Copy the given vector so as to make it the input
        //                      vector for the "i"th pattern in the pattern set,
        //                      returning the number of elements copied, or a 
        //                      negative value on error.
        int set_input_pattern(int i, gsl_vector* v);

        // target_pattern -- Copy the target vector for the "i"th pattern into the
        //                   given vector, returning a pointer to it.  Return NULL
        //                   if the vector is of the wrong size or if any other 
        //                   error arises.
        gsl_vector* target_pattern(int i, gsl_vector* v) const;

        // set_target_pattern -- Copy the given vector so as to make it the target
        //                       vector for the "i"th pattern in the pattern set,
        //                       returning the number of elements copied, or a 
        //                       negative value on error.
        int set_target_pattern(int i, gsl_vector* v);

        // read -- Fill the pattern set from the contents of the given input
        //         stream, returning the stream, setting the appropriate error
        //         bits on the stream when an error occurs.
        friend istream& operator>>(istream& istr, PatternSetRef& pset);

        // write -- Write the complete pattern set, one pattern per line, to
        //          the given output stream, returning the stream and setting
        //          the appropriate error bits on error.
        friend ostream& operator<<(ostream& ostr, const PatternSetRef& pset);

        // permute_patterns -- Fill the permutation array with a random ordering
        //                     of the pattern indices, allowing the entire
        //                     pattern set to be examined in a random order by
        //                     iteratively examining the pattern indexed by the
        //                     "t"th element of the permutation array.  Return
        //                     false on error.
        bool permute_patterns();

        // sort_euclidean -- Fill the permutation array so as to sort the input
        //                   vectors in order of increasing Euclidean distance from
        //                   the given reference vector.  Return false on error.
        bool sort_euclidean(gsl_vector* ref_v);

        // sort_angular -- Fill the permutation array so as to sort the input
        //                 vectors in order of increasing angular distance from
        //                 the given reference vector.  Return false on error.
        bool sort_angular(gsl_vector* ref_v);

        // pca_project -- Return a copy of this pattern set with all input
        //                vectors projected onto their principal component axes.
        //                The copy should be freshly allocated.  Return the
        //                original pattern set on error.
        PatternSetRef& pca_project();

};



#endif  // #ifndef PATTERNS_UTILITIES_INCLUDED





