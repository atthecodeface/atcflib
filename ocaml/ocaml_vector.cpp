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
 * It
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

/*f vector_of_val c_vector from an OCAML value
 */
#define vector_of_val(v) (*((c_vector **) Data_custom_val(v)))

/*f FN_CV_TO_FLOAT : c_vector -> float
 *
 * Function to return cv->fn() as a float
 *
 */
#define FN_CV_TO_FLOAT(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of vector %p\n", vector_of_val(v));     \
        CAMLreturn(caml_copy_double(vector_of_val(v)->fn()));        \
    }

/*f FN_CV_TO_INT : c_vector -> int
 *
 * Function to return cv->fn() as an int
 *
 */
#define FN_CV_TO_INT(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of vector %p\n", vector_of_val(v));     \
        CAMLreturn(Val_long(vector_of_val(v)->fn()));   \
    }

/*f FN_CV_TO_UNIT : c_vector -> unit
 *
 * Function to perform cv->fn() with no return value
 *
 */
#define FN_CV_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of vector %p\n", vector_of_val(v));     \
        vector_of_val(v)->fn();                                      \
        CAMLreturn0;                                                 \
    }

/*f FN_CV_CVR_TO_UNIT : c_vector -> c_vector -> unit
 *
 * Function to perform cv->fn(*c_vector) with no return value
 *
 */
#define FN_CV_CVR_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value v2) {  \
        CAMLparam2(v, v2);                                              \
        VERBOSE(stderr,#fn " of vector %p %p\n", vector_of_val(v), vector_of_val(v2));     \
        vector_of_val(v)->fn(*vector_of_val(v2));                        \
        CAMLreturn0;                                                    \
    }

/*f FN_CV_FLOAT_TO_UNIT : c_vector -> float -> unit
 *
 * Function to perform cv->fn(double) with no return value
 *
 */
#define FN_CV_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value f) {   \
        CAMLparam2(v,f);                                                \
        VERBOSE(stderr,#fn " of vector %p %f\n", vector_of_val(v), Double_val(f)); \
        vector_of_val(v)->fn(Double_val(f));                            \
        CAMLreturn0;                                                    \
    }

/*f FN_CV_INT_FLOAT_TO_UNIT : c_vector -> int -> float -> unit
 *
 * Function to perform cv->fn(int,double) with no return value
 *
 */
#define FN_CV_INT_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value n, value f) { \
        CAMLparam3(v,n,f);                                              \
        VERBOSE(stderr,#fn " of vector %p %ld %f\n", vector_of_val(v), Long_val(n), Double_val(f)); \
        vector_of_val(v)->fn(Long_val(n), Double_val(f));                \
        CAMLreturn0;                                                    \
    }

/*f FN_CV_CVR_FLOAT_TO_UNIT : c_vector -> c_vector -> float -> unit
 *
 * Function to perform cv->fn(*c_vector,double) with no return value
 *
 */
#define FN_CV_CVR_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value v2, value f) { \
        CAMLparam3(v,v2,f);                                             \
        VERBOSE(stderr,#fn " of vector %p %p %f\n", vector_of_val(v),  vector_of_val(v2), Double_val(f)); \
        vector_of_val(v)->fn(*vector_of_val(v2), Double_val(f));        \
        CAMLreturn0;                                                    \
    }

/*f FN_CV_CVR_TO_FLOAT : c_vector -> c_vector -> float
 *
 * Function to perform cv->fn(*c_vector) returning a float
 *
 */
#define FN_CV_CVR_TO_FLOAT(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        VERBOSE(stderr,#fn " of vector %p %p\n", vector_of_val(v),  vector_of_val(v2)); \
        CAMLreturn(caml_copy_double(vector_of_val(v)->fn(*vector_of_val(v2)))); \
    }

/*f FN_CV_CVR_TO_CV : c_vector -> c_vector -> c_vector
 *
 * Function to perform cv->fn(*c_vector) returning a NEW c_vector
 *
 */
#define FN_CV_CVR_TO_CV(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v, value v2) {  \
        CAMLparam2(v,v2);                                               \
        CAMLreturn(alloc_vector(vector_of_val(v)->fn(*vector_of_val(v2)))); \
    }

/*a Statics
 */
static struct custom_operations custom_ops = {
    (char *)"atcf.vector",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f alloc_vector
 *
 * Creates a vector from a NEW c_vector
 *
 */
static value
alloc_vector(c_vector *cv)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Created vector %p\n", cv);
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
    CAMLreturn(alloc_vector(new c_vector(Long_val(n))));
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
    CAMLreturn(alloc_vector(new c_vector(*vector_of_val(v))));
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
FN_CV_TO_INT(length)

/*f atcf_vector_modulus : c_vector -> float
 *
 * Return the vector modulus
 *
 */
FN_CV_TO_FLOAT(modulus)

/*f atcf_vector_modulus_squared : c_vector -> float
 *
 * Return the vector modulus squared
 *
 */
FN_CV_TO_FLOAT(modulus_squared)

/*a Assignment methods - side effects
 */
/*f atcf_vector_set : c_vector -> int -> float -> unit
 *
 * Set the nth coordinate to a value
 *
 */
FN_CV_INT_FLOAT_TO_UNIT(set)

/*f atcf_vector_assign : c_vector -> c_vector -> unit
 *
 * Assign the vector contents to be the contents of another vector
 *
 */
FN_CV_CVR_TO_UNIT(assign)

/*a Operations on the vector - have side effects
 */
/*f atcf_vector_scale : c_vector -> float -> unit
 *
 * Scale the vector by a factor
 *
 */
FN_CV_FLOAT_TO_UNIT(scale)

/*f atcf_vector_add_scaled : c_vector -> c_vector -> float -> unit
 *
 * vector <- vector + (vector2 * scale_factor)
 *
 */
FN_CV_CVR_FLOAT_TO_UNIT(add_scaled)

/*f atcf_vector_normalize : c_vector -> unit
 *
 * Normalize a vector (make it unit length)
 *
 * If the modulus of the vector is less than epsilon, then leave it unchanged
 *
 */
FN_CV_TO_UNIT(normalize)

/*a Operations that have no side-effects
 */
/*f atcf_vector_dot_product : c_vector -> c_vector -> float
 *
 * Return the inner product of two vectors
 *
 */
FN_CV_CVR_TO_FLOAT(dot_product)

/*f atcf_vector_cross_product3 : c_vector -> c_vector -> NEW c_vector
 *
 * NEW vector = v0 x v1, for length 3 vectors
 *
 */
FN_CV_CVR_TO_CV(cross_product3)

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
    VERBOSE(stderr,"Created vector %p\n", vector_of_val(vr));
    CAMLreturn(vr);
}

