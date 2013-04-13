#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([p3_driver], [1.0], [jlusby42@gmail.com])
AC_CONFIG_SRCDIR([p3_driver.cc])
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_INSTALL
AC_SEARCH_LIBS([gsl_atanh], [gsl], [], [], [-lgslcblas]) 
AC_SEARCH_LIBS([gsl_blas_ddot], [gslcblas], [], [], [-lgsl]) 
AC_TYPE_SIZE_T
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
