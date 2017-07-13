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

#include <atcf/quaternion.h>
#include <atcf/vector.h>
#include <atcf/matrix.h>

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

/*a Statics
 */
static void finalize_vector(value v)
{
    delete vector_of_val(v);
}

static struct custom_operations custom_ops = {
    (char *)"atcf.vector",
    finalize_vector,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f caml_atcf_alloc_vector
 *
 * Creates a vector from a NEW c_vector
 *
 */
extern value
caml_atcf_alloc_vector(c_vector *cv)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Allocked caml vector %p\n", cv);
    return v;
}

/*f atcf_vector_create : n:int -> NEW c_vector
 *
 * Creates a vector of length n
 *
 */
extern "C"
CAMLprim value
atcf_vector_create(value n)
{
    CAMLparam1(n);
    VERBOSE(stderr,"Create vector %ld\n",Long_val(n));
    CAMLreturn(caml_atcf_alloc_vector(new c_vector(Long_val(n))));
}


/*f atcf_vector_create : n:int -> NEW c_vector
 *
 * Creates a vector of length n
 *
 */
extern "C"
CAMLprim value
atcf_vector_create_bigarray_slice(value b, value l, value o, value s)
{
    CAMLparam4(b, l, o, s);
    int vl = Long_val(l);
    int vs = Long_val(s);
    int vo = Long_val(o);
    double *vb = (double *) Caml_ba_data_val(b);
    VERBOSE(stderr,"Create vector from bigarray data %p (%p:%d:%d) %d %d %d\n",
            vb, Caml_ba_array_val(b)->data,Caml_ba_array_val(b)->num_dims, Caml_ba_array_val(b)->dim[0],
            vl,vs,vo);
    CAMLreturn(caml_atcf_alloc_vector(new c_vector(vl,vs,vb+vo)));
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

/*f atcf_vector_clone : c_vector -> NEW c_vector
 *
 * Clones a vector
 *
 */
extern "C"
CAMLprim value
atcf_vector_clone(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Clone vector %p\n", vector_of_val(v));
    CAMLreturn(caml_atcf_alloc_vector(new c_vector(*vector_of_val(v))));
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
atcf_vector_coords(value v)
{
    CAMLparam1(v);
    c_vector *cv = vector_of_val(v);
    int n = cv->length();
    v = caml_alloc_float_array(n);
    for (int i=0; i<n; i++) {
        Store_double_field(v,i,cv->value(i));
    }
    CAMLreturn(v);
}

/*f atcf_vector_length : c_vector -> int
 *
 * Return the number of coordinates in the vector
 *
 */
FN_C_TO_INT(vector,length)

/*f atcf_vector_modulus : c_vector -> float
 *
 * Return the vector modulus
 *
 */
FN_C_TO_FLOAT(vector,modulus)

/*f atcf_vector_modulus_squared : c_vector -> float
 *
 * Return the vector modulus squared
 *
 */
FN_C_TO_FLOAT(vector,modulus_squared)

/*a Assignment methods - side effects
 */
/*f atcf_vector_set : c_vector -> int -> float -> unit
 *
 * Set the nth coordinate to a value
 *
 */
FN_C_INT_FLOAT_TO_UNIT(vector,set)

/*f atcf_vector_assign : c_vector -> c_vector -> unit
 *
 * Assign the vector contents to be the contents of another vector
 *
 */
FN_C_CR_TO_UNIT(vector,assign)

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
FN_C_FLOAT_TO_UNIT(vector,scale)

/*f atcf_vector_add_scaled : c_vector -> c_vector -> float -> unit
 *
 * vector <- vector + (vector2 * scale_factor)
 *
 */
FN_C_CR_FLOAT_TO_UNIT(vector,add_scaled)

/*f atcf_vector_normalize : c_vector -> unit
 *
 * Normalize a vector (make it unit length)
 *
 * If the modulus of the vector is less than epsilon, then leave it unchanged
 *
 */
FN_C_TO_UNIT(vector,normalize)

/*a Operations that have no side-effects
 */
/*f atcf_vector_dot_product : c_vector -> c_vector -> float
 *
 * Return the inner product of two vectors
 *
 */
FN_C_CR_TO_FLOAT(vector,dot_product)

/*f atcf_vector_cross_product3 : c_vector -> c_vector -> NEW c_vector
 *
 * NEW vector = v0 x v1, for length 3 vectors
 *
 */
FN_C_CR_TO_C(vector,cross_product3)

/*f atcf_vector_angle_axis_to3 : c_vector -> c_vector -> (NEW c_vector * float * float)
 *
 * Return the axis vector perpendicular to the two vectors, and the
 * cosine and sine of the angle require to rotate from the first to
 * the second on that axis.
 *
 */
extern "C"
CAMLprim value 
atcf_vector_angle_axis_to3(value v, value v2)
{
    CAMLparam2(v,v2);
    value vr = caml_alloc_tuple(3);
    value vrv = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    Store_field(vr,0,vrv);
    double cos, sin;
    c_vector *cv = vector_of_val(v)->angle_axis_to_v3(*vector_of_val(v2), &cos, &sin);
    vector_of_val(vrv) = cv;
    Store_field(vr,1,caml_copy_double(cos));
    Store_field(vr,2,caml_copy_double(sin));
    VERBOSE(stderr,"Created vector from angle_axis_to3 %p\n", vector_of_val(vr));
    CAMLreturn(vr);
}

