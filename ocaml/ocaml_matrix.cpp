/** Copyright (C) 2017,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          ocaml_matrix.cpp
 * @brief         Matrices and methods thereon
 *
 */

/*a Documentation
 * 
 * This is a wrapper around the ATCF library matrix class
 *
 * It provides ocaml wrappers for the matrix methods in 'matrix.h'
 *
 */
/*a Includes
 */
#define CAML_NAME_SPACE 
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/intext.h>
#include <caml/threads.h>

#include <atcf/vector.h>
#include <atcf/matrix.h>
#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_MATRIX_VERBOSE on compilation to be verbose,
// or uncomment the following
// #define __OCAML_MATRIX_VERBOSE
#ifdef __OCAML_MATRIX_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*f FN_CM_TO_FLOAT : c_matrix -> float
 *
 * Function to return cm->fn() as a float
 *
 */
#define FN_CM_TO_FLOAT(fn) \
    extern "C" CAMLprim value atcf_matrix_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of matrix %p\n", matrix_of_val(v));     \
        CAMLreturn(caml_copy_double(matrix_of_val(v)->fn()));        \
    }

/*f FN_CM_TO_INT : c_matrix -> int
 *
 * Function to return cm->fn() as an int
 *
 */
#define FN_CM_TO_INT(fn) \
    extern "C" CAMLprim value atcf_matrix_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of matrix %p\n", matrix_of_val(v));     \
        CAMLreturn(Val_long(matrix_of_val(v)->fn()));   \
    }

/*f FN_CM_TO_UNIT : c_matrix -> unit
 *
 * Function to perform cm->fn() with no return value
 *
 */
#define FN_CM_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_matrix_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of matrix %p\n", matrix_of_val(v));     \
        matrix_of_val(v)->fn();                                      \
        CAMLreturn0;                                                 \
    }

/*f FN_CM_CMR_TO_UNIT : c_matrix -> c_matrix -> unit
 *
 * Function to perform cm->fn(*c_matrix) with no return value
 *
 */
#define FN_CM_CMR_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_matrix_ ## fn (value v, value v2) {  \
        CAMLparam2(v, v2);                                              \
        VERBOSE(stderr,#fn " of matrix %p %p\n", matrix_of_val(v), matrix_of_val(v2));     \
        matrix_of_val(v)->fn(*matrix_of_val(v2));                        \
        CAMLreturn0;                                                    \
    }

/*f FN_CM_FLOAT_TO_UNIT : c_matrix -> float -> unit
 *
 * Function to perform cm->fn(double) with no return value
 *
 */
#define FN_CM_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_matrix_ ## fn (value v, value f) {   \
        CAMLparam2(v,f);                                                \
        VERBOSE(stderr,#fn " of matrix %p %f\n", matrix_of_val(v), Double_val(f)); \
        matrix_of_val(v)->fn(Double_val(f));                            \
        CAMLreturn0;                                                    \
    }

/*f FN_CM_INT_FLOAT_TO_UNIT : c_matrix -> int -> float -> unit
 *
 * Function to perform cm->fn(int,double) with no return value
 *
 */
#define FN_CM_INT_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_matrix_ ## fn (value v, value n, value f) { \
        CAMLparam3(v,n,f);                                              \
        VERBOSE(stderr,#fn " of matrix %p %ld %f\n", matrix_of_val(v), Long_val(n), Double_val(f)); \
        matrix_of_val(v)->fn(Long_val(n), Double_val(f));                \
        CAMLreturn0;                                                    \
    }

/*f FN_CM_INT_INT_FLOAT_TO_UNIT : c_matrix -> int -> int -> float -> unit
 *
 * Function to perform cm->fn(int,int,double) with no return value
 *
 */
#define FN_CM_INT_INT_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_matrix_ ## fn (value v, value r, value c, value f) { \
        CAMLparam4(v,r,c,f);                                            \
        VERBOSE(stderr,#fn " of matrix %p %ld %ld %f\n", matrix_of_val(v), Long_val(r), Long_val(c), Double_val(f)); \
        matrix_of_val(v)->fn(Long_val(r), Long_val(c), Double_val(f));                \
        CAMLreturn0;                                                    \
    }

/*f FN_CM_CMR_FLOAT_TO_UNIT : c_matrix -> c_matrix -> float -> unit
 *
 * Function to perform cm->fn(*c_matrix,double) with no return value
 *
 */
#define FN_CM_CMR_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_matrix_ ## fn (value v, value v2, value f) { \
        CAMLparam3(v,v2,f);                                             \
        VERBOSE(stderr,#fn " of matrix %p %p %f\n", matrix_of_val(v),  matrix_of_val(v2), Double_val(f)); \
        matrix_of_val(v)->fn(*matrix_of_val(v2), Double_val(f));        \
        CAMLreturn0;                                                    \
    }

/*f FN_CM_CMR_TO_FLOAT : c_matrix -> c_matrix -> float
 *
 * Function to perform cm->fn(*c_matrix) returning a float
 *
 */
#define FN_CM_CMR_TO_FLOAT(fn) \
    extern "C" CAMLprim value atcf_matrix_ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        VERBOSE(stderr,#fn " of matrix %p %p\n", matrix_of_val(v),  matrix_of_val(v2)); \
        CAMLreturn(caml_copy_double(matrix_of_val(v)->fn(*matrix_of_val(v2)))); \
    }

/*f FN_CM_CMR_TO_CM : c_matrix -> c_matrix -> c_matrix
 *
 * Function to perform cm->fn(*c_matrix) returning a NEW c_matrix
 *
 */
#define FN_CM_CMR_TO_CM(fn) \
    extern "C" CAMLprim value atcf_matrix_ ## fn (value v, value v2) {  \
        CAMLparam2(v,v2);                                               \
        CAMLreturn(alloc_matrix(matrix_of_val(v)->fn(*matrix_of_val(v2)))); \
    }

/*a Statics
 */
static struct custom_operations custom_ops = {
    (char *)"atcf.matrix",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f caml_atcf_alloc_matrix
 *
 * Creates a matrix from a NEW c_matrix
 *
 */
extern value
caml_atcf_alloc_matrix(c_matrix *cm)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_matrix *), 0, 1);
    matrix_of_val(v) = cm;
    VERBOSE(stderr,"Created matrix %p\n", cm);
    return v;
}

/*f atcf_matrix_create : r:int -> c:int -> NEW c_matrix
 *
 * Creates a matrix of length n
 *
 */
extern "C"
CAMLprim value
atcf_matrix_create(value r, value c)
{
    CAMLparam2(r,c);
    VERBOSE(stderr,"Create matrix %ld x %ld\n",Long_val(r),Long_val(c));
    CAMLreturn(caml_atcf_alloc_matrix(new c_matrix(Long_val(r),Long_val(c))));
}

/*f atcf_matrix_destroy : c_matrix -> unit
 *
 * Destroys a matrix
 *
 */
extern "C"
CAMLprim void
atcf_matrix_destroy(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Destroy matrix %p\n", matrix_of_val(v));
    delete matrix_of_val(v);
    matrix_of_val(v) = NULL;
    CAMLreturn0;
}

/*f atcf_matrix_clone : c_matrix -> NEW c_matrix
 *
 * Clones a matrix
 *
 */
extern "C"
CAMLprim value
atcf_matrix_clone(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Clone matrix %p\n", matrix_of_val(v));
    CAMLreturn(caml_atcf_alloc_matrix(new c_matrix(*matrix_of_val(v))));
}

/*a Interrogation functions - not effecting the c_matrix
 */
/*f atcf_matrix_row_vector : c_matrix -> int -> c_vector
 *
 * Return the nth row-vector of the matrix
 *
 */
extern "C"
CAMLprim value
atcf_matrix_row_vector(value v, value r)
{
    CAMLparam2(v,r);
    c_matrix *cm = matrix_of_val(v);
    CAMLreturn(caml_atcf_alloc_vector(cm->get_row(Long_val(r))));
}

/*f atcf_matrix_col_vector : c_matrix -> int -> c_vector
 *
 * Return the nth col-vector of the matrix
 *
 */
extern "C"
CAMLprim value
atcf_matrix_col_vector(value v, value r)
{
    CAMLparam2(v,r);
    c_matrix *cm = matrix_of_val(v);
    CAMLreturn(caml_atcf_alloc_vector(cm->get_column(Long_val(r))));
}

/*f atcf_matrix_nrows : c_matrix -> int
 *
 * Return the number of rows in the matrix
 *
 */
FN_CM_TO_INT(nrows)

/*f atcf_matrix_ncols : c_matrix -> int
 *
 * Return the number of rows in the matrix
 *
 */
FN_CM_TO_INT(ncols)

/*a Setting functions */
FN_CM_TO_UNIT(set_identity)
FN_CM_INT_INT_FLOAT_TO_UNIT(set)

/*f atcf_vector_assign_m_v
  Assign value to be that of matrix m applied to other vector v2
 */
extern "C"
CAMLprim void
atcf_matrix_assign_m_m(value m, value m1, value m2)
{
    CAMLparam3(m, m1, m2);
    matrix_of_val(m)->multiply(*matrix_of_val(m1),*matrix_of_val(m2));
    CAMLreturn0;
}

/*a Operation functions */
/*f apply : c_matrix -> c_vector -> NEW c_vector
 *
 * Function to perform m * v -> new vector
 *
 */
extern "C"
CAMLprim value
atcf_matrix_apply(value m, value v) {
    CAMLparam2(m, v);
    CAMLreturn(caml_atcf_alloc_vector(matrix_of_val(m)->apply(*vector_of_val(v))));
}

/*f scale : c_matrix -> float -> unit */
FN_CM_FLOAT_TO_UNIT(scale)

/*f add_scaled : c_matrix -> c_matrix -> float -> unit */
FN_CM_CMR_FLOAT_TO_UNIT(add_scaled)

/*f transpose
 */
FN_CM_TO_UNIT(transpose)

/*f lup_get_l, lup_get_u
 */
FN_CM_TO_UNIT(lup_get_l)
FN_CM_TO_UNIT(lup_get_u)

/*f lup_decompose : c_matrix -> c_vector
 *
 * Function to perform m->lup_decompose -> new vector
 *
 */
extern "C"
CAMLprim value
atcf_matrix_lup_decompose(value m) {
    CAMLparam1(m);
    c_vector *v;
    if (matrix_of_val(m)->lup_decompose(&v)) { // raise exception
    }
    CAMLreturn(caml_atcf_alloc_vector(v));
}

/*f lup_invert : c_matrix -> unit, in-place inversion
 *
 * Function to perform m->lup_invert
 *
 */
extern "C"
CAMLprim void
atcf_matrix_lup_invert(value m) {
    CAMLparam1(m);
    c_vector *v;
    if (matrix_of_val(m)->lup_decompose(&v)) { // raise exception
    }
    matrix_of_val(m)->lup_invert(*v);
    delete v;
    CAMLreturn0;
}

/*f lup_inverse : c_matrix -> c_matrix
 *
 * Function to perform m->lup_inverse, and return the new matrix
 *
 */
FN_CM_TO_UNIT(lup_inverse)
