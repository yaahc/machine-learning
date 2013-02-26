//
// patterns.cc :  Implementation file for a "pattern set" object.
//
// David C. Noelle - Mon Feb 23 15:09:44 PST 2009
// 


#include <iostream>
#include <iomanip>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_sort_double.h>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_eigen.h>

#include "patterns.h"


//
// PatternSet Class  --  Member function implementations
//

// constructors

PatternSet::PatternSet(int num_pat, int num_inputs, int num_targets) {
  n_patterns = num_pat;
  n_inputs = num_inputs;
  n_targets = num_targets;
  // permutation sequence default values ...
  if (n_patterns > 0) {
    permute = true;
    if (permutation = new int[n_patterns])
      // initialize permutation sequence ...
      for (int i = 0; i < n_patterns; i++)
	permutation[i] = i;
  } else {
    permute = false;
    permutation = NULL;
  }
  // allocate input vector storage ...
  if ((n_patterns > 0) && (n_inputs > 0)) {
    inputs_m = gsl_matrix_alloc(n_patterns, n_inputs);
  } else {
    inputs_m = NULL;
  }
  // allocate target vector storage ...
  if ((n_patterns > 0) && (n_targets > 0)) {
    targets_m = gsl_matrix_alloc(n_patterns, n_targets);
  } else {
    targets_m = NULL;
  }
  // check for allocation failure ...
  if ((n_patterns > 0) &&
      ((permutation == NULL) ||
       ((n_inputs > 0) && (inputs_m == NULL)) ||
       ((n_targets > 0) && (targets_m == NULL)))) {
    // invalidate this pattern set ...
    n_patterns = -1;
    n_inputs = -1;
    n_targets = -1;
  }
}

PatternSet::PatternSet(const PatternSet& pset) {
  n_patterns = pset.n_patterns;
  n_inputs = pset.n_inputs;
  n_targets = pset.n_targets;
  // permutation sequence ...
  permute = pset.permute;
  permutation = NULL;
  if ((n_patterns > 0) && (pset.permutation != NULL))
    if (permutation = new int[n_patterns])
      // copy permutation sequence ...
      for (int i = 0; i < n_patterns; i++)
	permutation[i] = pset.permutation[i];
  // allocate input vector storage and copy input vectors ...
  if ((n_patterns > 0) && (n_inputs > 0) && (pset.inputs_m != NULL)) {
    if (inputs_m = gsl_matrix_alloc(n_patterns, n_inputs))
      (void) gsl_matrix_memcpy(inputs_m, pset.inputs_m);
  } else {
    inputs_m = NULL;
  }
  // allocate target vector storage and copy target vectors ...
  if ((n_patterns > 0) && (n_targets > 0) && (pset.targets_m != NULL)) {
    if (targets_m = gsl_matrix_alloc(n_patterns, n_targets))
      (void) gsl_matrix_memcpy(targets_m, pset.targets_m);
  } else {
    targets_m = NULL;
  }
  // check for allocation failure ...
  if ((n_patterns > 0) &&
      ((permutation == NULL) ||
       ((n_inputs > 0) && (inputs_m == NULL)) ||
       ((n_targets > 0) && (targets_m == NULL)))) {
    // invalidate this pattern set ...
    n_patterns = -1;
    n_inputs = -1;
    n_targets = -1;
  }
}


// assignment

PatternSet& PatternSet::operator=(const PatternSet& pset) {
  int old_n_patterns;

  // Do nothing if we are assigning a pattern set to itself ...
  if (this != &pset) {
    old_n_patterns = n_patterns;
    n_patterns = pset.n_patterns;
    n_inputs = pset.n_inputs;
    n_targets = pset.n_targets;
    // copy input vector values, making sure sizes are appropriate ...
    if (pset.inputs_m != NULL) {
      if ((inputs_m->size1 != pset.inputs_m->size1) ||
	  (inputs_m->size2 != pset.inputs_m->size2)) {
	gsl_matrix_free(inputs_m);
	inputs_m 
	  = gsl_matrix_alloc(pset.inputs_m->size1, pset.inputs_m->size2);
      }
      (void) gsl_matrix_memcpy(inputs_m, pset.inputs_m);
    } else {
      if (inputs_m) {
	gsl_matrix_free(inputs_m);
	inputs_m = NULL;
      }
    }
    // copy target vector values, making sure sizes are appropriate ...
    if (pset.targets_m != NULL) {
      if ((targets_m->size1 != pset.targets_m->size1) ||
	  (targets_m->size2 != pset.targets_m->size2)) {
	gsl_matrix_free(targets_m);
	targets_m 
	  = gsl_matrix_alloc(pset.targets_m->size1, pset.targets_m->size2);
      }
      (void) gsl_matrix_memcpy(targets_m, pset.targets_m);
    } else {
      if (targets_m) {
	gsl_matrix_free(targets_m);
	targets_m = NULL;
      }
    }
    // copy permutation array values, making sure sizes are correct ...
    if (pset.permutation != NULL) {
      // Use the previous value of "n_patterns" as our only indication
      // of the length of the permutation array ...
      if (old_n_patterns != n_patterns) {
	delete [] permutation;
	permutation = new int[n_patterns];
      }
      for (int i = 0; i < n_patterns; i++)
	permutation[i] = pset.permutation[i];
    } else {
      if (permutation) {
	delete [] permutation;
	permutation = NULL;
      }
    }
    permute = pset.permute;
  }
  return *this;
}


// destructor

PatternSet::~PatternSet() {
  n_patterns = -1;
  n_inputs = -1;
  n_targets = -1;
  permute = false;
  // deallocate the input vectors ...
  if (inputs_m) {
    gsl_matrix_free(inputs_m);
    inputs_m = NULL;
  }
  // deallocate the target vectors ...
  if (targets_m) {
    gsl_matrix_free(targets_m);
    targets_m = NULL;
  }
  // deallocate the permutation array ...
  if (permutation) {
    delete [] permutation;
    permutation = NULL;
  }
}


// get_permuted_i -- Return the index, "i", of the pattern at position "t"
//                   in the permuted order specified by the permutation
//                   array.  Return "t" if the permutation array is not
//                   properly initialized or the permute flag is false.
//                   Return a negative value on error.

int PatternSet::get_permuted_i(int t) const {
  if ((t >= 0) && (t < n_patterns)) {
    if (permutation)
      return permutation[t];
    else
      return (t);
  } else {
    return (-1);
  }
}


// full_pattern -- Copy the pattern with the given index (the "i"th pattern)
//                 into the given vector, returning a pointer to the 
//                 vector.  Return NULL on error.

gsl_vector* PatternSet::full_pattern(int i, gsl_vector* v) const {
  // Some of these error tests might be considered overly conservative ...
  if (((i >= 0) && (i < n_patterns)) &&
      (v != NULL) && (v->size == n_inputs + n_targets)) {
    // copy input pattern into the given vector ...
    for (int j = 0; j < n_inputs; j++)
      gsl_vector_set(v, j, gsl_matrix_get(inputs_m, i, j));
    // copy target pattern into the given vector ...
    for (int j = 0; j < n_targets; j++)
      gsl_vector_set(v, j + n_inputs, gsl_matrix_get(targets_m, i, j));
    // Return result ...
    return (v);
  } else {
    return (NULL);
  }
}


// input_pattern -- Copy the input vector for the "i"th pattern into the 
//                  given vector, returning a pointer to it.  Return NULL
//                  if the vector is of the wrong size or if any other
//                  error arises.

gsl_vector* PatternSet::input_pattern(int i, gsl_vector* v) const {
  if ((i >= 0) && (i < n_patterns) &&
      (v != NULL) && (v->size == n_inputs)) {
    (void) gsl_matrix_get_row(v, inputs_m, i);
    return (v);
  } else {
    return (NULL);
  }
}

    
// set_input_pattern -- Copy the given vector so as to make it the input
//                      vector for the "i"th pattern in the pattern set,
//                      returning the number of elements copied, or a 
//                      negative value on error.

int PatternSet::set_input_pattern(int i, gsl_vector* v) {
  if ((i >= 0) && (i < n_patterns) &&
      (v != NULL) && (v->size == n_inputs)) {
    (void) gsl_matrix_set_row(inputs_m, i, v);
    return (n_inputs);
  } else {
    return (-1);
  }
}


// target_pattern -- Copy the target vector for the "i"th pattern into the
//                   given vector, returning a pointer to it.  Return NULL
//                   if the vector is of the wrong size or if any other 
//                   error arises.

gsl_vector* PatternSet::target_pattern(int i, gsl_vector* v) const {
  if ((i >= 0) && (i < n_patterns) &&
      (v != NULL) && (v->size == n_targets)) {
    (void) gsl_matrix_get_row(v, targets_m, i);
    return (v);
  } else {
    return (NULL);
  }
}


// set_target_pattern -- Copy the given vector so as to make it the target
//                       vector for the "i"th pattern in the pattern set,
//                       returning the number of elements copied, or a 
//                       negative value on error.

int PatternSet::set_target_pattern(int i, gsl_vector* v) {
  if ((i >= 0) && (i < n_patterns) &&
      (v != NULL) && (v->size == n_targets)) {
    (void) gsl_matrix_set_row(targets_m, i, v);
    return (n_targets);
  } else {
    return (-1);
  }
}


// read -- Fill the pattern set from the contents of the given input
//         stream, returning the stream, setting the appropriate error
//         bits on the stream when an error occurs.

istream& operator>>(istream& istr, PatternSet& pset) {
  double value;  // temporary buffer

  if (pset.inputs_m || pset.targets_m) {
    // Note that testing "istr" is the same as checking the "failbit" ...
    for (int i = 0; (i < pset.n_patterns) && istr; i++) {
      for (int j_in = 0; (j_in < pset.n_inputs) && istr; j_in++) {
	istr >> value;
	gsl_matrix_set(pset.inputs_m, i, j_in, value);
      }
      for (int j_targ = 0; (j_targ < pset.n_targets) && istr; j_targ++) {
	istr >> value;
	gsl_matrix_set(pset.targets_m, i, j_targ, value);
      }
    }
  }
  return (istr);
}


// write -- Write the complete pattern set, one pattern per line, to
//          the given output stream, returning the stream and setting
//          the appropriate error bits on error.

ostream& operator<<(ostream& ostr, const PatternSet& pset) {
  int pat_i;     // real pattern index

  if (pset.inputs_m || pset.targets_m) {
    // Note that testing "ostr" is the same as checking the "failbit" ...
    for (int i = 0; (i < pset.n_patterns) && ostr; i++) {
      if (pset.permute) {
	pat_i = pset.get_permuted_i(i);
      } else {
	pat_i = i;
      }
      for (int j_in = 0; (j_in < pset.n_inputs) && ostr; j_in++) {
	ostr << " " << setw(14) << fixed << setprecision(8) 
	     <<	gsl_matrix_get(pset.inputs_m, pat_i, j_in);
      }
      for (int j_targ = 0; (j_targ < pset.n_targets) && ostr; j_targ++) {
	ostr << " " << setw(14) << fixed << setprecision(8) 
	     <<	gsl_matrix_get(pset.targets_m, pat_i, j_targ);
      }
      ostr << endl;
    }
  }
  return (ostr);
}


// permute_patterns -- Fill the permutation array with a random ordering
//                     of the pattern indices, allowing the entire
//                     pattern set to be examined in a random order by
//                     iteratively examining the pattern indexed by the
//                     "t"th element of the permutation array.  Return
//                     false on error.

bool PatternSet::permute_patterns() {
  int swap_i;
  int temp_value;

  if (permutation) {
    // initialize the random number generator ...
    (void) gsl_rng_env_setup();
    // use the random number algorithm specified by the GSL_RNG_TYPE 
    // environment variable ...
    const gsl_rng_type* T = gsl_rng_default;
    gsl_rng* rand_generator = gsl_rng_alloc(T);
    // Note that the random number generator is randomly seeded by the 
    // value of the GSL_RNG_SEED enviornment variable, or zero, by default.

    // assume that the permutation array is currently of the appropriate
    // size and contains all of the integers from 0 to one less than
    // "n_patterns", in some order ...
    for (int i = 0; i < n_patterns; i++) {
      // pick a random place in the array to swap places with the "i"th
      // element ...
      swap_i = (int) gsl_rng_uniform_int(rand_generator, n_patterns);
      // perform the swap ...
      if (i != swap_i) {
	temp_value = permutation[swap_i];
	permutation[swap_i] = permutation[i];
	permutation[i] = temp_value;
      }
    }
    gsl_rng_free(rand_generator);
    return true;
  } else {
    return false;
  }
}


// sort_euclidean -- Fill the permutation array so as to sort the input
//                   vectors in order of increasing Euclidean distance from
//                   the given reference vector.  Return false on error.

bool PatternSet::sort_euclidean(gsl_vector* ref_v) {
  if (permutation) {
    double* distances = new double[n_patterns];
    size_t* perm = new size_t[n_patterns];
    gsl_vector* diff_v = gsl_vector_alloc(n_inputs);

    if (distances && perm && diff_v) {
      for (int i = 0; i < n_patterns; i++) {
	(void) gsl_matrix_get_row(diff_v, inputs_m, i);
	(void) gsl_vector_sub(diff_v, ref_v);
	distances[i] = gsl_blas_dnrm2(diff_v);
      }
      (void) gsl_sort_index(perm, distances, 1, n_patterns);
      for (int i = 0; i < n_patterns; i++)
	permutation[i] = (int) perm[i];
      // deallocate storage ...
      delete [] distances;
      delete [] perm;
      gsl_vector_free(diff_v);
      // return success ...
      return (true);
    } else {
      if (distances)
	delete [] distances;
      if (perm)
	delete [] perm;
      if (diff_v)
	gsl_vector_free(diff_v);
      // return failure due to allocation problems ...
      return (false);
    }
  } else {
    // return failure ...
    return (false);
  }
}


// sort_angular -- Fill the permutation array so as to sort the input
//                 vectors in order of increasing angular distance from
//                 the given reference vector.  Return false on error.

bool PatternSet::sort_angular(gsl_vector* ref_v) {
  if (permutation) {
    double* distances = new double[n_patterns];
    size_t* perm = new size_t[n_patterns];
    double dot_product;
    double pattern_vector_length;

    if (distances && perm) {
      for (int i = 0; i < n_patterns; i++) {
	gsl_vector_view pat_v_view = gsl_matrix_row(inputs_m, i);
	(void) gsl_blas_ddot(&pat_v_view.vector, ref_v, &dot_product);
	pattern_vector_length = gsl_blas_dnrm2(&pat_v_view.vector);
	if (pattern_vector_length > 0) {
	  // Note that we don't bother to divide by the length of the 
	  // reference vector, since it is the same length for every
	  // pattern ...
	  distances[i] = dot_product / pattern_vector_length;
	} else {
	  // Zero length vectors are taken as orthogonal to all reference
	  // vectors ...
	  distances[i] = 0.0;
	}
      }
      (void) gsl_sort_index(perm, distances, 1, n_patterns);
      // The "perm" array is now sorted in increasing order of *inner product*,
      // which is exactly the opposite of the order we want.  (Large inner
      // products happen for small angles.)  We reverse the order when we move
      // the results into the "permutation" array ...
      for (int i = 0; i < n_patterns; i++)
	permutation[i] = perm[n_patterns - i - 1];
      // deallocate storage ...
      delete [] distances;
      delete [] perm;
      // return success ...
      return (true);
    } else {
      if (distances)
	delete [] distances;
      if (perm)
	delete [] perm;
      // return failure due to allocation problems ...
      return (false);
    }
  } else {
    // return failure ...
    return (false);
  }
}


// covariance_matrix -- This utility function returns the covariance
//                      matrix associated with the given matrix of
//                      pattern vectors.  Storage for the matrix is
//                      passed in as the first argument.  The mean of
//                      each data feature is passed in as a vector.
//                      Return NULL on error.

gsl_matrix* covariance_matrix(gsl_matrix* cov_m, gsl_matrix* data_m, 
			      gsl_vector* means_v) {
  int n;             // number of data vectors
  int d;             // dimensionality of data space
  double cov_value;  // a single entry in the covariance matrix

  if (cov_m && data_m) {
    n = data_m->size1;
    d = data_m->size2;
    if ((n > 0) && (d > 0) && (d == cov_m->size1) && (d == cov_m->size2)) {
      for (int j1 = 0; j1 < d; j1++)
	for (int j2 = 0; j2 <= j1; j2++) {
	  // calculate the covariance between data dimension "j1" and
	  // data dimension "j2" ...
	  gsl_vector_view col1_v_view = gsl_matrix_column(data_m, j1);
	  gsl_vector_view col2_v_view = gsl_matrix_column(data_m, j2);
	  cov_value = gsl_stats_covariance_m(col1_v_view.vector.data,
					     col1_v_view.vector.stride,
					     col2_v_view.vector.data,
					     col2_v_view.vector.stride,
					     n,
					     gsl_vector_get(means_v, j1),
					     gsl_vector_get(means_v, j2));
	  gsl_matrix_set(cov_m, j1, j2, cov_value);
	  gsl_matrix_set(cov_m, j2, j1, cov_value);
	}
      return (cov_m);
    } else {
      return (NULL);
    }
  } else {
    return (NULL);
  }
}


// pca_project -- Return a copy of this pattern set with all input
//                vectors projected onto their principal component axes.
//                The copy should be freshly allocated.  Return the
//                original pattern set on error.

PatternSet& PatternSet::pca_project() {
  if ((n_patterns > 0) && (n_inputs > 0)) {
    // compute the vector element means ...
    gsl_vector* means_v = gsl_vector_alloc(n_inputs);
    for (int j = 0; j < n_inputs; j++) {
      gsl_vector_view col_v_view = gsl_matrix_column(inputs_m, j);
      gsl_vector_set(means_v, j, 
		     gsl_stats_mean(col_v_view.vector.data, 
				    col_v_view.vector.stride, n_patterns));
    }
    // compute the covariance matrix ...
    gsl_matrix* cov_m = gsl_matrix_alloc(n_inputs, n_inputs);
    cov_m = covariance_matrix(cov_m, inputs_m, means_v);
    // compute the eigenvectors of the covariance matrix ...
    gsl_eigen_symmv_workspace* ws = gsl_eigen_symmv_alloc(n_inputs);
    gsl_vector* eigenvalues = gsl_vector_alloc(n_inputs);
    gsl_matrix* eigenvectors = gsl_matrix_alloc(n_inputs, n_inputs);
    (void) gsl_eigen_symmv(cov_m, eigenvalues, eigenvectors, ws);
    // sort the eigenvalues from low to high (reverse order) ...
    size_t* eval_perm = new size_t[n_inputs];
    (void) gsl_sort_index(eval_perm, eigenvalues->data, 
			  eigenvalues->stride, n_inputs);
    // make a copy of the pattern set, using the copy constructor ...
    PatternSet* new_pset = new PatternSet(*this);
    // allocate storage for an orginal pattern vector and the corresponding
    // projected vector ...
    gsl_vector* pat_v = gsl_vector_alloc(n_inputs);
    double projected_value;
    gsl_vector* projected_v = gsl_vector_alloc(n_inputs);
    // iterate over the input vectors ...
    for (int pat_i = 0; pat_i < n_patterns; pat_i++) {
      // extract the pattern input vector ...
      (void) gsl_matrix_get_row(pat_v, inputs_m, pat_i);
      // offset pattern vector by the distribution mean ...
      (void) gsl_vector_sub(pat_v, means_v);
      // iterate over the eigenvectors, in order ...
      for (int eval_i = 0; eval_i < n_inputs; eval_i++) {
	// find the "i"th principal component, noting that "eval_perm" is
	// sorted from low eigenvalues to high eigenvalues (reverse order) ...
	int pca_i = eval_perm[n_inputs - eval_i - 1];
	gsl_vector_view pca_i_v_view = gsl_matrix_column(eigenvectors, pca_i);
	// project this input vector onto the "i"th principal component ...
	(void) gsl_blas_ddot(pat_v, &pca_i_v_view.vector, &projected_value);
	// record the projected coordinate ...
	gsl_vector_set(projected_v, eval_i, projected_value);
      }
      // record the projected vector into the new pattern set ...
      (void) new_pset->set_input_pattern(pat_i, projected_v);
    }
    // deallocate storage ...
    gsl_vector_free(means_v);
    gsl_matrix_free(cov_m);
    gsl_eigen_symmv_free(ws);
    gsl_vector_free(eigenvalues);
    gsl_matrix_free(eigenvectors);
    delete [] eval_perm;
    gsl_vector_free(pat_v);
    gsl_vector_free(projected_v);
    // return new pattern set ...
    return (*new_pset);
  } else {
    return (*this);
  }
}

