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
 * @file          ocaml_vector.cpp
 * @brief         Vectors and methods thereon
 *
 */

/*a Documentation
 * 
 * This is a wrapper around the ATCF library vector class
 *
 * It provides ocaml wrappers for the vector methods in 'vector.h'
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
#include <caml/bigarray.h>

#include "quaternion.h"
#include "vector.h"
#include "matrix.h"

#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_VECTOR_VERBOSE on compilation to be verbose,
// or uncomment the following
// #define __OCAML_VECTOR_VERBOSE
#ifdef __OCAML_VECTOR_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*a Creation functions
 */
extern value caml_atcf_alloc_vector(class c_vector<double> *cv)
{
    return Val_unit;
}

/*f atcf_v_create_bigarray_slice : n:int -> NEW c_vector
 *
 * Creates a vector of length n
 *
 */
extern "C"
CAMLprim value
atcf_v_create_bigarray_slice(value ba, value l, value o, value s)
{
    CAMLparam4(ba, l, o, s);
    CAMLlocal1 (result);
    int vl = Long_val(l);
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
    CAMLreturn(result);
}

/*f atcf_vector_destroy : c_vector -> unit
 *
 * Destroys a vector
 *
 */
extern "C"
CAMLprim void
atcf_vector_destroy(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Destroy vector %p\n", vector_of_val(v));
    delete vector_of_val(v);
    vector_of_val(v) = NULL;
    CAMLreturn0;
}

/*a Interrogation functions - not effecting the c_vector
 */
/*f atcf_vector_coords : c_vector -> float array
 *
 * Return an array containing the coordinates of the vector
 *
 */
extern "C"
CAMLprim value
atcf_v_coords(value v)
{
    CAMLparam1(v);
    CAMLlocal1(result);
    c_vector<double> *cv = vector_of_val(v);
    int n = cv->length();
    result = caml_alloc_float_array(n);
    for (int i=0; i<n; i++) {
        Store_double_field(result,i,cv->get(i));
    }
    CAMLreturn(result);
}

/*f atcf_vector_length : c_vector -> int
 *
 * Return the number of coordinates in the vector
 *
 */
FN_MO_C_TO_INT(v,length)

/*f atcf_vector_modulus : c_vector -> float
 *
 * Return the vector modulus
 *
 */
FN_MO_C_TO_FLOAT(v,modulus)

/*f atcf_vector_modulus_squared : c_vector -> float
 *
 * Return the vector modulus squared
 *
 */
FN_MO_C_TO_FLOAT(v,modulus_squared)

/*a Assignment methods - side effects
 */
/*f atcf_vector_get : c_vector -> int -> float
 *
 * Get the nth coordinate
 *
 */
FN_MO_C_INT_TO_FLOAT(v,get)

/*f atcf_vector_set : c_vector -> int -> float -> unit
 *
 * Set the nth coordinate to a value
 *
 */
FN_MO_C_INT_FLOAT_TO_UNIT(v,set)

/*f atcf_vector_assign : c_vector -> c_vector -> unit
 *
 * Assign the vector contents to be the contents of another vector
 *
 */
FN_MO_C_CR_TO_UNIT(v,assign)

/*f atcf_vector_assign_m_v
  Assign value to be that of matrix m applied to other vector v2
 */
extern "C"
CAMLprim void
atcf_vector_assign_m_v(value v, value m, value v2)
{
    CAMLparam3(v, m, v2);
    matrix_of_val(m)->apply(*vector_of_val(v2),
                            *vector_of_val(v));
    CAMLreturn0;
}

/*f atcf_vector_assign_q : vector -> quaternion -> float * float
 * Assign value to be the axis of quaternion,
 * and return (cos, sin) as a float tuple
 */
extern "C"
CAMLprim value
atcf_vector_assign_q(value v, value q)
{
    double cos, sin;
    CAMLparam2(v, q);
    CAMLlocal3 (vr, cr, sr);
    quaternion_of_val(q)->as_rotation(*vector_of_val(v), &cos, &sin);
    cr = caml_copy_double(cos);
    sr = caml_copy_double(sin);
    vr = caml_alloc_tuple(2);
    Field(vr, 0) = cr;
    Field(vr, 1) = sr;
    CAMLreturn(vr);
}

/*f atcf_vector_apply_q : vector -> quaternion -> unit
 * Apply quaternion q to the vector (rotate it, most likely...)
 */
extern "C"
CAMLprim void
atcf_vector_apply_q(value v, value q)
{
    CAMLparam2(v, q);
    quaternion_of_val(q)->rotate_vector(vector_of_val(v));
    CAMLreturn0;
}

/*a Operations on the vector - have side effects
 */
/*f atcf_vector_scale : c_vector -> float -> unit
 *
 * Scale the vector by a factor
 *
 */
FN_MO_C_FLOAT_TO_UNIT(v,scale)

/*f atcf_vector_add_scaled : c_vector -> c_vector -> float -> unit
 *
 * vector <- vector + (vector2 * scale_factor)
 *
 */
FN_MO_C_CR_FLOAT_TO_UNIT(v,add_scaled)

/*f atcf_vector_normalize : c_vector -> unit
 *
 * Normalize a vector (make it unit length)
 *
 * If the modulus of the vector is less than epsilon, then leave it unchanged
 *
 */
FN_MO_C_TO_UNIT(v,normalize)

/*a Operations that have no side-effects
 */
/*f atcf_vector_dot_product : c_vector -> c_vector -> float
 *
 * Return the inner product of two vectors
 *
 */
FN_MO_C_CR_TO_FLOAT(v,dot_product)

/*f atcf_vector_cross_product3 : c_vector -> c_vector -> NEW c_vector
 *
 * NEW vector = v0 x v1, for length 3 vectors
 *
 */
//FN_C_CR_TO_C(vector,cross_product3)

/*f atcf_vector_angle_axis_to3 : c_vector -> c_vector -> (NEW c_vector * float * float)
 *
 * Return the axis vector perpendicular to the two vectors, and the
 * cosine and sine of the angle require to rotate from the first to
 * the second on that axis.
 *
 */
extern "C"
CAMLprim value 
atcf_vector_angle_axis_to3(value v, value v2, value rv)
{
    CAMLparam3(v,v2, rv);
    CAMLlocal1(result);
    if (math_obj_validate(v, MT_V_DOUBLE, 3, 3) &&
        math_obj_validate(v2, MT_V_DOUBLE, 3, 3) &&
        math_obj_validate(rv, MT_V_DOUBLE, 3, 3)) {
        result = caml_alloc_tuple(3);
        double cos, sin;
        (void) vector_of_val(v)->angle_axis_to_v3(*vector_of_val(v2), &cos, &sin, vector_of_val(rv));
        Store_field(result,0,rv);
        Store_field(result,1,caml_copy_double(cos));
        Store_field(result,2,caml_copy_double(sin));
    }
    CAMLreturn(result);
}

