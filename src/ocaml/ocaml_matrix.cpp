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

#include "vector.h"
#include "matrix.h"
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

/*f atcf_m_of_bigarray : n:int -> NEW c_vector
 *
 * Creates a vector of length n
 *
 */
extern "C"
CAMLprim value
atcf_m_of_bigarray(value ba, value l, value o, value s)
{
    CAMLparam4(ba, l, o, s);
    CAMLlocal1 (result);
/*    int vl = Long_val(l);
    int vs = Long_val(s);
    int vo = Long_val(o);
    struct caml_ba_array *cba = Caml_ba_array_val(ba);
    intnat size = cba->dim[0]; // assume one-dimensional since that is what we require
    if (vs<0) vs = 1;
    if (vo<0) vo = 0;
    if (vl<0) vl = size;
    if (vs*vl+vo>size) { vs=1; vo=0; vl=size; }

    t_math_type mt;
    void *vector;
    
    if ((cba->flags & CAML_BA_KIND_MASK)==CAML_BA_FLOAT64) {
        double *vb = (double *) Caml_ba_data_val(ba);
        vector = (void *)new c_vector<double>(vl,vs,vb+vo);
        mt = MT_V_DOUBLE;
    } else {
        float *vb = (float *) Caml_ba_data_val(ba);
        vector = (void *)new c_vector<float>(vl,vs,vb+vo);
        mt = MT_V_FLOAT;
    }
    VERBOSE(stderr,"Create vector from bigarray data %d %p (%p:%d:%d) %d %d %d\n", mt,
            Caml_ba_array_val(ba)->data,Caml_ba_array_val(ba)->num_dims, Caml_ba_array_val(ba)->dim[0],
            vl,vs,vo);

    caml_atcf_alloc_math_obj(&result, mt, &ba, vector);
*/    CAMLreturn(result);
}

/*f atcf_matrix_create : r:int -> c:int -> NEW c_matrix
 *
 * Creates a matrix of length n
 *
 */
extern "C"
CAMLprim value
atcf_m_create(value r, value c)
{
    fprintf(stderr, "Remove me atcf_m_create\n");
    CAMLparam2(r,c);
    CAMLlocal1 (result);
    int nr = Long_val(r);
    int nc = Long_val(c);
    int rs = 0;
    int cs = 0;
    int vo = 0;
    VERBOSE(stderr,"Create matrix %ld x %ld\n",Long_val(r),Long_val(c));

    t_math_type mt;
    void *matrix;

    if (1) { //(cba->flags & CAML_BA_KIND_MASK)==CAML_BA_FLOAT64) {
        //double *vb = (double *) Caml_ba_data_val(ba);
        double *vb = (double *)malloc(sizeof(double)*nr*nc);
        for (int i=0; i<nr*nc; i++) vb[i]=0;
        matrix = (void *)new c_matrix<double>(nr,nc,vb+vo,rs,cs);
        mt = MT_M_DOUBLE;
    } else {
        //float *vb = (float *) Caml_ba_data_val(ba);
        float *vb = (float *)malloc(sizeof(float)*nr*nc);
        for (int i=0; i<nr*nc; i++) vb[i]=0;
        matrix = (void *)new c_matrix<float>(nr,nc,vb+vo,rs,cs);
        mt = MT_M_FLOAT;
    }
    caml_atcf_alloc_math_obj(&result, mt, &r, matrix); // should be &ba
    CAMLreturn(result);
}

/*a Interrogation functions - not effecting the c_matrix
 */
/*f atcf_m_row_vector : c_matrix -> int -> c_vector -> unit
 *
 * Return the nth row-vector of the matrix
 *
 */
extern "C"
CAMLprim void
atcf_m_row_vector(value m, value r, value v)
{
    CAMLparam3(m,r,v);
    t_math_obj *mm = math_obj_of_val(m);
    t_math_obj *mv = math_obj_of_val(v);
    if (math_obj_of_double(mm)) {
        mm->ptr.md->get_row(Long_val(r), mv->ptr.vd);
    } else {
        mm->ptr.mf->get_row(Long_val(r), mv->ptr.vf);
    }
    CAMLreturn0;
}

/*f atcf_m_col_vector : c_matrix -> int -> c_vector
 *
 * Return the nth col-vector of the matrix
 *
 */
extern "C"
CAMLprim void
atcf_m_col_vector(value m, value r, value v)
{
    CAMLparam3(m,r,v);
    t_math_obj *mm = math_obj_of_val(m);
    t_math_obj *mv = math_obj_of_val(v);
    if (math_obj_of_double(mm)) {
        mm->ptr.md->get_column(Long_val(r), mv->ptr.vd);
    } else {
        mm->ptr.mf->get_column(Long_val(r), mv->ptr.vf);
    }
    CAMLreturn0;
}

/*f atcf_m_nrows : c_matrix -> int
 *
 * Return the number of rows in the matrix
 *
 */
FN_MO_C_TO_INT(m, nrows)

/*f atcf_m_ncols : c_matrix -> int
 *
 * Return the number of rows in the matrix
 *
 */
FN_MO_C_TO_INT(m, ncols)

/*a Setting functions */
FN_MO_C_TO_UNIT(m, set_identity)
FN_MO_C_INT_INT_FLOAT_TO_UNIT(m, set)

/*f atcf_m_assign
  Assign value to be that of matrix m1
 */
extern "C"
CAMLprim void
atcf_m_assign(value m, value m1)
{
    CAMLparam2(m, m1);
    t_math_obj *mm = math_obj_of_val(m);
    t_math_obj *mm1 = math_obj_of_val(m1);
    if (math_obj_of_double(mm)) {
        mm->ptr.md->scale(0);
        mm->ptr.md->add_scaled(*mm1->ptr.md,1);
    } else {
        mm->ptr.mf->scale(0);
        mm->ptr.mf->add_scaled(*mm1->ptr.mf,1);
    }
    CAMLreturn0;
}

/*f atcf_m_assign_m_m
  Assign value to be that of matrix m applied to other vector v2
 */
extern "C"
CAMLprim void
atcf_m_assign_m_m(value m, value m1, value m2)
{
    CAMLparam3(m, m1, m2);
    t_math_obj *mm = math_obj_of_val(m);
    t_math_obj *mm1 = math_obj_of_val(m1);
    t_math_obj *mm2 = math_obj_of_val(m2);
    if (math_obj_of_double(mm)) {
        mm->ptr.md->multiply(*mm1->ptr.md,*mm2->ptr.md);
    } else {
        mm->ptr.mf->multiply(*mm1->ptr.mf,*mm2->ptr.mf);
    }
    CAMLreturn0;
}

/*a Operation functions */
/*f atcf_m_apply : c_matrix -> c_vector -> NEW c_vector
 *
 * Function to perform m * v -> new vector
 *
 */
extern "C"
CAMLprim void
atcf_m_apply(value m, value v, value rv) {
    CAMLparam3(m, v, rv);
    t_math_obj *mm  = math_obj_of_val(m);
    t_math_obj *mv  = math_obj_of_val(v);
    t_math_obj *mrv = math_obj_of_val(rv);
    if (math_obj_of_double(mm)) {
        mm->ptr.md->apply(*mv->ptr.vd, *mrv->ptr.vd);
    } else {
        mm->ptr.mf->apply(*mv->ptr.vf, *mrv->ptr.vf);
    }
    CAMLreturn0;
}

/*f scale : c_matrix -> float -> unit */
FN_MO_C_FLOAT_TO_UNIT(m, scale)

/*f add_scaled : c_matrix -> c_matrix -> float -> unit */
FN_MO_C_CR_FLOAT_TO_UNIT(m, add_scaled)

/*f transpose_data
 */
FN_MO_C_TO_UNIT(m, transpose_data)

/*f lup_get_l, lup_get_u
 */
FN_MO_C_TO_UNIT(m, lup_get_l)
FN_MO_C_TO_UNIT(m, lup_get_u)

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

/*f lup_decompose : c_matrix -> c_vector -> unit
 *
 * Function to perform m->lup_decompose -> new vector
 *
 */
extern "C"
CAMLprim void
atcf_m_lup_decompose(value m, value v) {
    CAMLparam2(m, v);
    t_math_obj *mm  = math_obj_of_val(m);
    t_math_obj *mv  = math_obj_of_val(v);
    if (math_obj_of_double(mm)) {
        if (mm->ptr.md->lup_decompose(&mv->ptr.vd)) { // raise exception
            caml_failwith("Failed to decompose");
        }
    } else {
        if (mm->ptr.mf->lup_decompose(&mv->ptr.vf)) { // raise exception
            caml_failwith("Failed to decompose");
        }
    }
    CAMLreturn0;
}

/*f lup_invert : c_matrix -> unit, in-place inversion
 *
 * Function to perform m->lup_invert
 *
 */
extern "C"
CAMLprim void
atcf_m_lup_invert(value m) {
    CAMLparam1(m);
    t_math_obj *mm  = math_obj_of_val(m);
    if (math_obj_of_double(mm)) {
        c_vector<double> *v = NULL;
        if (mm->ptr.md->lup_decompose(&v)) { // raise exception
            caml_failwith("Failed to invert");
        } else {
            mm->ptr.md->lup_invert(*v);
            delete v;
        }
    } else {
        c_vector<float> *v = NULL;
        if (mm->ptr.mf->lup_decompose(&v)) { // raise exception
            caml_failwith("Failed to invert");
        } else {
            mm->ptr.mf->lup_invert(*v);
            delete v;
        }
    }
    CAMLreturn0;
}

