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
#define MAKING_MATRIX
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

/*a Statics
 */
static void finalize_matrix(value v)
{
    delete matrix_of_val(v);
}

static struct custom_operations custom_ops = {
    (char *)"atcf.matrix",
    finalize_matrix,
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
caml_atcf_alloc_matrix(c_matrix<double> *cm)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_matrix<double> *), 0, 1);
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
    CAMLreturn(caml_atcf_alloc_matrix(new c_matrix<double>(Long_val(r),Long_val(c))));
}

/*f atcf_matrix_destroy : c_matrix -> unit
 *
 * Destroys a matrix
 *
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
 */

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
    CAMLreturn(caml_atcf_alloc_matrix(new c_matrix<double>(*matrix_of_val(v))));
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
    c_matrix<double> *cm = matrix_of_val(v);
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
    c_matrix<double> *cm = matrix_of_val(v);
    CAMLreturn(caml_atcf_alloc_vector(cm->get_column(Long_val(r))));
}

/*f atcf_matrix_nrows : c_matrix -> int
 *
 * Return the number of rows in the matrix
 *
 */
FN_C_TO_INT(matrix, nrows)

/*f atcf_matrix_ncols : c_matrix -> int
 *
 * Return the number of rows in the matrix
 *
 */
FN_C_TO_INT(matrix, ncols)

/*a Setting functions */
FN_C_TO_UNIT(matrix, set_identity)
FN_C_INT_INT_FLOAT_TO_UNIT(matrix, set)

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
FN_C_FLOAT_TO_UNIT(matrix, scale)

/*f add_scaled : c_matrix -> c_matrix -> float -> unit */
FN_C_CR_FLOAT_TO_UNIT(matrix, add_scaled)

/*f transpose_data
 */
FN_C_TO_UNIT(matrix, transpose_data)

/*f lup_get_l, lup_get_u
 */
FN_C_TO_UNIT(matrix, lup_get_l)
FN_C_TO_UNIT(matrix, lup_get_u)

/*f assign_from_q : c_matrix -> c_quaternion
 *
 * Function to assign m to be the rotation due to q
 *
 */
extern "C"
CAMLprim void
atcf_matrix_assign_from_q(value m, value q) {
    CAMLparam2(m, q);
    matrix_of_val(m)->set_from_quaternion(*quaternion_of_val(q));
    CAMLreturn0;
}

/*f lup_decompose : c_matrix -> c_vector
 *
 * Function to perform m->lup_decompose -> new vector
 *
 */
extern "C"
CAMLprim value
atcf_matrix_lup_decompose(value m) {
    CAMLparam1(m);
    c_vector<double> *v;
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
    c_vector<double> *v;
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
FN_C_TO_UNIT(matrix, lup_inverse)
