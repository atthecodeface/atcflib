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
 * @file          ocaml_quaternion.cpp
 * @brief         Quaternions and methods thereon
 *
 */

/*a Documentation
 * 
 * This is a wrapper around the ATCF library quaternion class
 *
 * It provides ocaml wrappers for the quaternion methods in 'quaternion.h'
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
#include <atcf/quaternion.h>
#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_QUATERNION_VERBOSE on compilation to be verbose,
// or uncomment the following
//#define __OCAML_QUATERNION_VERBOSE
#ifdef __OCAML_QUATERNION_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*a Statics
 */
static void finalize_quaternion(value v)
{
    delete quaternion_of_val(v);
}

static struct custom_operations custom_ops = {
    (char *)"atcf.quaternion",
    finalize_quaternion,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f caml_atcf_alloc_quaternion
 *
 * Creates a quaternion from a NEW c_quaternion
 *
 */
extern value
caml_atcf_alloc_quaternion(c_quaternion<double> *cm)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_quaternion<double> *), 0, 1);
    quaternion_of_val(v) = cm;
    VERBOSE(stderr,"Created quaternion %p\n", cm);
    return v;
}

/*f atcf_quaternion_create : r:unit -> NEW c_quaternion
 *
 * Creates a quaternion zero
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_create(void)
{
    CAMLparam0();
    VERBOSE(stderr,"Create quaternion\n");
    CAMLreturn(caml_atcf_alloc_quaternion(new c_quaternion<double>()));
}

/*f atcf_quaternion_create_rijk : r:float -> i:float ...  -> NEW c_quaternion
 *
 * Creates a quaternion with r, i, j, k
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_create_rijk(value r, value i, value j, value k)
{
    CAMLparam4(r,i,j,k);
    VERBOSE(stderr,"Create quaternion\n");
    c_quaternion<double> *q=new c_quaternion<double>(Double_val(r),
                                     Double_val(i),
                                     Double_val(j),
                                     Double_val(k));
    CAMLreturn(caml_atcf_alloc_quaternion(q));
}

/*f atcf_quaternion_destroy : c_quaternion -> unit
 *
 * Destroys a quaternion
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_destroy(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Destroy quaternion %p\n", quaternion_of_val(v));
    delete quaternion_of_val(v);
    quaternion_of_val(v) = NULL;
    CAMLreturn0;
}

/*f atcf_quaternion_clone : c_quaternion -> NEW c_quaternion
 *
 * Clones a quaternion
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_clone(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Clone quaternion %p\n", quaternion_of_val(v));
    CAMLreturn(caml_atcf_alloc_quaternion(new c_quaternion<double>(*quaternion_of_val(v))));
}

/*a Assignment functions
 */
/*f atcf_quaternion_assign_q : c_quaternion -> c_quaternion -> unit
 *
 * Assign to value of other quaternion
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_assign_q(value q, value q2)
{
    CAMLparam2(q, q2);
    *(quaternion_of_val(q)) = *quaternion_of_val(q2);
    CAMLreturn0;
}

/*f atcf_quaternion_assign_lookat_graphics : c_quaternion -> c_vector -> c_vector -> unit
 *
 * Assign a quaternion rotation that looks at 'at' with 'up' as up
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_assign_lookat_graphics(value q, value at, value up)
{
    CAMLparam3(q, at, up);
    quaternion_of_val(q)->lookat_graphics(*vector_of_val(at),
                                          *vector_of_val(up));
                                 
    CAMLreturn0;
}

/*f atcf_quaternion_assign_lookat_aeronautic : c_quaternion -> c_vector -> c_vector -> unit
 *
 * Assign a quaternion rotation that looks at 'at' with 'up' as up
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_assign_lookat_aeronautic(value q, value at, value up)
{
    CAMLparam3(q, at, up);
    quaternion_of_val(q)->lookat_aeronautic(*vector_of_val(at),
                                            *vector_of_val(up));
                                 
    CAMLreturn0;
}

/*f atcf_quaternion_assign_of_rotation : c_quaternion -> c_vector -> float -> float -> unit
 *
 * Assign a quaternion rotation with axis and cos/sin
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_assign_of_rotation(value q, value axis, value c, value s)
{
    CAMLparam4(q, axis, c, s);
    quaternion_of_val(q)->from_rotation(Double_val(c),
                                        Double_val(s),
                                        *vector_of_val(axis));
    CAMLreturn0;
}

/*a Interrogation functions
 */
/*f atcf_quaternion_rijk : c_quaternion -> float array
 *
 * Return an array containing the coordinates of the vector
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_rpy(value q)
{
    double rpy[3];
    CAMLparam1(q);
    c_quaternion<double> *cq = quaternion_of_val(q);
    value v = caml_alloc_float_array(3);
    cq->as_euler(rpy);
    for (int i=0; i<3; i++) 
        Store_double_field(v,i,rpy[i]);
    CAMLreturn(v);
}

/*f atcf_quaternion_rijk : c_quaternion -> float array
 *
 * Return an array containing the coordinates of the vector
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_rijk(value q)
{
    double rijk[4];
    CAMLparam1(q);
    c_quaternion<double> *cq = quaternion_of_val(q);
    value v = caml_alloc_float_array(4);
    cq->get_rijk(rijk);
    for (int i=0; i<4; i++) 
        Store_double_field(v,i,rijk[i]);
    CAMLreturn(v);
}

/*f atcf_quaternion_modulus : c_quaternion -> float 
 *
 * Return the modulus of a quaternion
 *
 */
FN_C_TO_FLOAT(quaternion, modulus);

/*f atcf_quaternion_modulus_squared : c_quaternion -> float 
 *
 * Return the square of the modulus of a quaternion
 *
 */
FN_C_TO_FLOAT(quaternion, modulus_squared);

/*f atcf_quaternion_add_scaled : c_quaternion -> float -> unit 
 *
 *
 *
 */
FN_C_CR_FLOAT_TO_UNIT(quaternion,add_scaled)
FN_C_CR_TO_FLOAT(quaternion, distance_to);

/*a Operation functions
 */
FN_C_TO_UNIT(quaternion, conjugate);
FN_C_TO_UNIT(quaternion, reciprocal);
FN_C_TO_UNIT(quaternion, normalize);
FN_C_FLOAT_TO_UNIT(quaternion, scale);

/*f atcf_quaternion_postmultiply : c_quaternion -> c_quaternion -> unit
 *
 * Post-multiply by another quaternion
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_postmultiply(value q, value q2)
{
    CAMLparam2(q, q2);
    quaternion_of_val(q)->multiply(*quaternion_of_val(q2), 0);
    CAMLreturn0;
}

/*f atcf_quaternion_premultiply : c_quaternion -> c_quaternion -> unit
 *
 * Pre-multiply by another quaternion
 *
 */
extern "C"
CAMLprim void
atcf_quaternion_premultiply(value q, value q2)
{
    CAMLparam2(q, q2);
    quaternion_of_val(q)->multiply(*quaternion_of_val(q2), 1);
    CAMLreturn0;
}

