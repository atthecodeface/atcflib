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
#include <caml/bigarray.h>

#include "vector.h"
#include "matrix.h"
#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_MATRIX_VERBOSE on compilation to be verbose,
// or uncomment the following
//#define __OCAML_MATRIX_VERBOSE
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
    //delete matrix_of_val(v);
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
/*f atcf_m_of_bigarray : ba -> offset -> size_stride:int array -> NEW c_vector
 *
 * Creates a matrix from bigarray data starting at offset
 * size_stride must be an array of size 4 of ncols, col stride, nrows, row stride
 *
 */
extern "C"
CAMLprim value
atcf_m_of_bigarray(value ba, value o, value ss_array)
{
    CAMLparam3(ba, o, ss_array);
    CAMLlocal1 (result);
    int vo = Long_val(o);
    struct caml_ba_array *cba = Caml_ba_array_val(ba);
    int ncols  = Long_val(Field(ss_array, 0));
    int colstr = Long_val(Field(ss_array, 1));
    int nrows  = Long_val(Field(ss_array, 2));
    int rowstr = Long_val(Field(ss_array, 3));
    // caml_array_length(ss_array) == 4;   /* size in items */
    intnat size = cba->dim[0]; // assume one-dimensional since that is what we require
    if (ncols<=0) {ncols=1;}
    if (nrows<=0) {nrows=1;}
    if (colstr<=0) {colstr=nrows;}
    if (rowstr<=0) {rowstr=ncols;}
    if ((ncols*nrows)>size) { ncols=1; nrows=1; }
    if (vo<0) {vo=0;}
    if (vo>=size) {vo=0;}
    int index;
    index = (ncols-1)*colstr+vo;
    if ((index<0) || (index>=size)) { vo=0; colstr=1; rowstr=1; }
    index = (nrows-1)*rowstr+vo;
    if ((index<0) || (index>=size)) { vo=0; colstr=1; rowstr=1; }
    index = (ncols-1)*colstr+(nrows-1)*rowstr+vo;
    if ((index<0) || (index>=size)) { vo=0; colstr=1; rowstr=1; }

    t_math_type mt;
    void *matrix;
    
    if ((cba->flags & CAML_BA_KIND_MASK)==CAML_BA_FLOAT64) {
        double *vb = (double *) Caml_ba_data_val(ba);
        matrix = (void *)new c_matrix<double>(nrows, ncols, vb+vo, rowstr, colstr);
        mt = MT_M_DOUBLE;
    } else {
        float *vb = (float *) Caml_ba_data_val(ba);
        matrix = (void *)new c_matrix<float>(nrows, ncols, vb+vo, rowstr, colstr);
        mt = MT_M_FLOAT;
    }
    VERBOSE(stderr,"Create matrix from bigarray data type %d ba (%p:%ld:%ld) %d %d %d %d %d\n", mt,
            Caml_ba_array_val(ba)->data,Caml_ba_array_val(ba)->num_dims, Caml_ba_array_val(ba)->dim[0],
            vo,nrows,ncols,rowstr,colstr);

    caml_atcf_alloc_math_obj(&result, mt, &ba, vo, matrix);
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
    VERBOSE(stderr,"Get row vector of mm %p into mv %p data types %d %d\n", mm, mv, mm->mt, mv->mt);
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
    VERBOSE(stderr,"Get col vector of mm %p into mv %p data types %d %d\n", mm, mv, mm->mt, mv->mt);
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
  Assign value of m to be that of matrix m1
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
        mm->ptr.md->add_scaled(*mm1->ptr.cmd,1);
    } else {
        mm->ptr.mf->scale(0);
        mm->ptr.mf->add_scaled(*mm1->ptr.cmf,1);
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
    VERBOSE(stderr,"Multiply mv %p by mm %p in to mrv %p types %d %d %d\n", mv, mm, mrv, mv->mt, mm->mt, mrv->mt);
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
    t_math_obj *mm = math_obj_of_val(m);
    VERBOSE(stderr,"Assign mm %p from quaterion type %d\n", mm, mm->mt );
    if (math_obj_of_double(mm)) {
        mm->ptr.md->set_from_quaternion(*quaternion_of_val(q));
    } else {
        fprintf(stderr, "atcf_matrix_assign_from_q(value m, value q) { not written yet\n");
        //mm->ptr.mf->set_from_quaternion(*quaternion_of_val(q));
    }
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
    VERBOSE(stderr,"LUP decompose mm %p type %d ba data %p\n", mm, mm->mt, Caml_ba_data_val(mm->ba) );
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

