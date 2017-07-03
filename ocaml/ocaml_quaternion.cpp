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
#include <atcf/quaternion.h>
#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_QUATERNION_VERBOSE on compilation to be verbose,
// or uncomment the following
// #define __OCAML_QUATERNION_VERBOSE
#ifdef __OCAML_QUATERNION_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*a Statics
 */
static struct custom_operations custom_ops = {
    (char *)"atcf.quaternion",
    custom_finalize_default,
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
caml_atcf_alloc_quaternion(c_quaternion *cm)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_quaternion *), 0, 1);
    quaternion_of_val(v) = cm;
    VERBOSE(stderr,"Created quaternion %p\n", cm);
    return v;
}

/*f atcf_quaternion_create : r:int -> c:int -> NEW c_quaternion
 *
 * Creates a quaternion of length n
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_create(void)
{
    CAMLparam0();
    VERBOSE(stderr,"Create quaternion\n");
    CAMLreturn(caml_atcf_alloc_quaternion(new c_quaternion()));
}

/*f atcf_quaternion_create : r:int -> c:int -> NEW c_quaternion
 *
 * Creates a quaternion of length n
 *
 */
extern "C"
CAMLprim value
atcf_quaternion_create_rijk(value r, value i, value j, value k)
{
    CAMLparam4(r,i,j,k);
    VERBOSE(stderr,"Create quaternion\n");
    c_quaternion *q=new c_quaternion(Long_val(r),
                                     Long_val(i),
                                     Long_val(j),
                                     Long_val(k));
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
    CAMLreturn(caml_atcf_alloc_quaternion(new c_quaternion(*quaternion_of_val(v))));
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
    c_quaternion *cq = quaternion_of_val(q);
    value v = caml_alloc_float_array(4);
    cq->get_rijk(rijk);
    for (int i=0; i<4; i++) 
        Store_double_field(v,i,rijk[i]);
    CAMLreturn(v);
}

FN_C_TO_UNIT(quaternion, conjugate);
FN_C_TO_UNIT(quaternion, reciprocal);
FN_C_TO_UNIT(quaternion, normalize);
FN_C_FLOAT_TO_UNIT(quaternion, scale);
FN_C_TO_FLOAT(quaternion, modulus);
FN_C_TO_FLOAT(quaternion, modulus_squared);
FN_C_CR_FLOAT_TO_UNIT(quaternion,add_scaled)
FN_C_CR_TO_FLOAT(quaternion, distance_to);
